// ---------- Arduino UNO code (LAB LOGIC VERSION) ----------
// Sends MPU1, MPU2, MPU3(logical), DHT22, SOIL, RAIN to ESP via SoftwareSerial

#include <Wire.h>
#include <SoftwareSerial.h>
#include <math.h>
#include <DHT.h>

// --------- I2C Addresses (physical) ----------
const uint8_t MPU1_ADDR = 0x68;  // Deep layer
const uint8_t MPU2_ADDR = 0x69;  // Middle layer

// --------- SoftwareSerial to ESP ----------
// Arduino D2 (RX), D3 (TX)
SoftwareSerial espSerial(2, 3);

// --------- DHT22 on Arduino D7 ----------
#define DHTPIN 7
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// --------- Soil & Rain Sensors ----------
#define SOIL_PIN A0
#define RAIN_PIN A1

// --------- Constants ----------
const float gScale = 16384.0;  // 1g = 16384 LSB at ±2g

// Previous accel values for vibration
int16_t prevAx[2] = {0, 0};
int16_t prevAy[2] = {0, 0};
int16_t prevAz[2] = {0, 0};
bool    firstRead[2] = {true, true};

// Store last pitch & DAE for each physical MPU
float lastPitch[2] = {0, 0};
long  lastDAE[2]   = {0, 0};
float DAE[2]       = {0, 0};

// For DHT timing
unsigned long lastDhtMillis = 0;
const unsigned long DHT_INTERVAL = 2000; // read every 2 seconds

// Rain accumulation
static float rain_mm_total = 0;       // total mm since boot (approx)
static float rain_mm_lastHour = 0;    // approx last hour rain
static unsigned long lastRainStep = 0;


// ---------- Low-level I2C helpers ----------

// Wake up MPU6050 (clear sleep bit in PWR_MGMT_1)
void wakeMPU(uint8_t addr) {
  Wire.beginTransmission(addr);
  Wire.write(0x6B);   // PWR_MGMT_1
  Wire.write(0x00);   // set to 0 => awake
  Wire.endTransmission();
  delay(50);
}

// Read raw accel (AX, AY, AZ) from given address
bool readAccel(uint8_t addr, int16_t &ax, int16_t &ay, int16_t &az) {
  Wire.beginTransmission(addr);
  Wire.write(0x3B);   // ACCEL_XOUT_H
  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  Wire.requestFrom(addr, (uint8_t)6);
  if (Wire.available() < 6) {
    return false;
  }

  ax = (Wire.read() << 8) | Wire.read();
  ay = (Wire.read() << 8) | Wire.read();
  az = (Wire.read() << 8) | Wire.read();

  return true;
}

// ---------- Calculations ----------
float computePitch(int16_t ax, int16_t ay, int16_t az) {
  float axg = (float)ax / gScale;
  float ayg = (float)ay / gScale;
  float azg = (float)az / gScale;
  float pitch = atan2(axg, sqrt(ayg * ayg + azg * azg)) * 180.0 / PI;
  return pitch;
}

// Advanced tilt classification for lab
String classifyTiltAdvanced(float pitch) {
  float a = fabs(pitch);
  if (a < 8)        return "Stable";
  else if (a < 20)  return "MinorTilt";
  else if (a < 35)  return "MajorTilt";
  else              return "SlopeShift";
}

// Advanced vibration classification using DAE (Delta Accel Energy)
String classifyVibrationFromDAE(long dae) {
  if (dae < 100)        return "None";
  else if (dae < 500)   return "Low";
  else if (dae < 2000)  return "Medium";
  else                  return "High";
}

// index = 0 for MPU1 (deep), 1 for MPU2 (middle)
void processMPU(uint8_t index, uint8_t addr) {
  int16_t ax, ay, az;
  bool ok = readAccel(addr, ax, ay, az);

  if (!ok) {
    Serial.print("MPU");
    Serial.print(index + 1);
    Serial.println(" read FAILED");
    return;
  }

  float pitch = computePitch(ax, ay, az);

  long delta = 0;
  if (!firstRead[index]) {
    delta =
      labs((long)ax - prevAx[index]) +
      labs((long)ay - prevAy[index]) +
      labs((long)az - prevAz[index]);
  }

  // Update historical accel
  prevAx[index] = ax;
  prevAy[index] = ay;
  prevAz[index] = az;
  firstRead[index] = false;

  // Delta Acceleration Energy (smoothing)
  DAE[index] = (DAE[index] * 0.6f) + (delta * 0.4f);

  // Save for virtual top sensor logic
  lastPitch[index] = pitch;
  lastDAE[index]   = (long)DAE[index];

  // Classify tilt & vibration
  String tiltLevel = classifyTiltAdvanced(pitch);
  String vibLevel  = classifyVibrationFromDAE((long)DAE[index]);

  // Depth-based special labels for lab explanation
  if (index == 0 && vibLevel == "High") {
    vibLevel = "DeepMovement";   // deep layer movement
  }
  if (index == 1 && tiltLevel == "SlopeShift") {
    tiltLevel = "MidSlopeShift"; // movement in middle layer
  }

  // Build text line to send
  // Example: MPU1,12.3,Stable,350,Low
  String line = "MPU";
  line += String(index + 1);
  line += ",";
  line += String(pitch, 1);
  line += ",";
  line += tiltLevel;
  line += ",";
  line += String((long)DAE[index]);
  line += ",";
  line += vibLevel;

  Serial.println(line);      // debug to PC
  espSerial.println(line);   // send to ESP
}

// ---------- Virtual TOP layer MPU (MPU3) ----------
// This is a logical sensor combining deep + middle
void processVirtualTopMPU() {
  float pitchTop = (lastPitch[0] * 0.4f) + (lastPitch[1] * 0.6f);
  long daeTop = max(lastDAE[0], lastDAE[1]);

  String tiltLevel = classifyTiltAdvanced(pitchTop);
  String vibLevel  = classifyVibrationFromDAE(daeTop);

  // Surface specific labels
  if (tiltLevel == "SlopeShift") {
    tiltLevel = "SurfaceSlide";
  }
  if (vibLevel == "High") {
    vibLevel = "SurfaceShock";
  }

  String line = "MPU3,";
  line += String(pitchTop, 1);
  line += ",";
  line += tiltLevel;
  line += ",";
  line += String(daeTop);
  line += ",";
  line += vibLevel;

  Serial.println(line);
  espSerial.println(line);
}

// ---------- Read DHT22 and send ----------
void processDHT() {
  unsigned long now = millis();
  if (now - lastDhtMillis < DHT_INTERVAL) return; // wait 2 seconds
  lastDhtMillis = now;

  float h = dht.readHumidity();
  float t = dht.readTemperature(); // Celsius

  if (isnan(h) || isnan(t)) {
    Serial.println("DHT read FAILED");
    return;
  }

  // Example line: DHT,28.5,65.2
  String line = "DHT,";
  line += String(t, 1);
  line += ",";
  line += String(h, 1);

  Serial.println(line);
  espSerial.println(line);
}

// ---------- Soil Moisture ADVANCED LOGIC ----------
void processSoil() {
  int raw = analogRead(SOIL_PIN);

  int percent = map(raw, 1023, 300, 0, 100);  // adjust 300 if needed
  percent = constrain(percent, 0, 100);

  // Moving average (smoothing)
  static float soilAvg = 0;
  soilAvg = (soilAvg * 0.7f) + (percent * 0.3f);

  // Hydrological saturation index
  static int saturationIndex = 0;
  if (percent > 60) saturationIndex += 2;
  else if (percent > 40) saturationIndex += 1;
  else if (percent < 30 && saturationIndex > 0) saturationIndex -= 1;

  saturationIndex = constrain(saturationIndex, 0, 20);

  String level;
  if (soilAvg < 25) {
    level = "Dry";
  } else if (soilAvg < 45) {
    if (saturationIndex < 5) level = "Moist";
    else                     level = "Moist-Risk";
  } else if (soilAvg < 65) {
    if (saturationIndex < 10) level = "Wet";
    else                      level = "HighSaturation";
  } else {
    if (saturationIndex < 12) level = "VeryWet";
    else                      level = "CriticalSaturation"; // landslide trigger zone
  }

  // Example: SOIL,45,Moist
  String line = "SOIL,";
  line += String(percent);
  line += ",";
  line += level;

  Serial.println(line);
  espSerial.println(line);
}

// ---------- Rain Sensor ADVANCED LOGIC (with pseudo-precipitation) ----------
void processRain() {
  int rainRaw = analogRead(RAIN_PIN);
  int rainPercent = map(rainRaw, 1023, 300, 0, 100);
  rainPercent = constrain(rainPercent, 0, 100);

  unsigned long now = millis();
  if (now - lastRainStep < 10000) {
    // only calculate every 10 seconds
    return;
  }
  lastRainStep = now;

  // ----- Intensity band -----
  String intensity;
  if (rainPercent < 10)       intensity = "NoRain";
  else if (rainPercent < 30)  intensity = "Drizzle";
  else if (rainPercent < 50)  intensity = "Light";
  else if (rainPercent < 75)  intensity = "Moderate";
  else                        intensity = "Heavy";

  // ----- Map intensity -> mm/hour (approx, lab logic) -----
  float mmPerHour = 0;
  if (intensity == "NoRain")        mmPerHour = 0;
  else if (intensity == "Drizzle")  mmPerHour = 1;
  else if (intensity == "Light")    mmPerHour = 3;
  else if (intensity == "Moderate") mmPerHour = 8;
  else if (intensity == "Heavy")    mmPerHour = 20;

  // mm for this 10-second step
  float mmThisStep = mmPerHour / 360.0;   // 360 * 10s = 1 hour

  // accumulate
  rain_mm_total += mmThisStep;

  // last hour (exponential decay model)
  rain_mm_lastHour = rain_mm_lastHour * 0.95f + mmThisStep;

  // ----- Risk logic based on last-hour rain -----
  String risk;
  if (rain_mm_lastHour < 2)        risk = "Very Low";
  else if (rain_mm_lastHour < 10)  risk = "Low";
  else if (rain_mm_lastHour < 25)  risk = "Moderate";
  else if (rain_mm_lastHour < 50)  risk = "High";
  else                             risk = "Extreme";

  // ----- Build line to send -----
  // Format: RAIN,<percent>,<intensity>,<mmLastHour>,<risk>
  String line = "RAIN,";
  line += String(rainPercent);
  line += ",";
  line += intensity;
  line += ",";
  line += String(rain_mm_lastHour, 2);
  line += ",";
  line += risk;

  Serial.println(line);
  espSerial.println(line);
}

void setup() {
  Serial.begin(9600);       // PC debug
  espSerial.begin(9600);    // must match ESP side
  Wire.begin();             // SDA=A4, SCL=A5 on UNO

  dht.begin();

  delay(1000);
  Serial.println("Arduino: LAB sensors running...");
  Serial.println("Waking MPUs 0x68 (deep) and 0x69 (middle)...");

  wakeMPU(MPU1_ADDR);
  wakeMPU(MPU2_ADDR);

  Serial.println("Setup done.\n");
}

void loop() {
  // Physical MPUs
  processMPU(0, MPU1_ADDR);  // Deep
  delay(10);
  processMPU(1, MPU2_ADDR);  // Middle
  delay(10);

  // Virtual top layer MPU3
  processVirtualTopMPU();
  delay(10);

  // Other sensors
  processDHT();              // every 2 seconds (internal timing)
  processSoil();
  delay(10);
  processRain();

  delay(300);
}
