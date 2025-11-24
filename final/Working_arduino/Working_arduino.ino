// ---------- Arduino UNO code ----------
// Sends MPU1, MPU2, DHT22, SOIL, RAIN to ESP via SoftwareSerial

#include <Wire.h>
#include <SoftwareSerial.h>
#include <math.h>
#include <DHT.h>

// --------- I2C Addresses ----------
const uint8_t MPU1_ADDR = 0x68;  // MPU1: AD0 = GND
const uint8_t MPU2_ADDR = 0x69;  // MPU2: AD0 = VCC

// --------- SoftwareSerial to ESP ----------
SoftwareSerial espSerial(2, 3);  // RX, TX  (Arduino D2, D3)

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

// For DHT timing
unsigned long lastDhtMillis = 0;
const unsigned long DHT_INTERVAL = 2000; // read every 2 seconds

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

String classifyTilt(float pitch) {
  float a = fabs(pitch);
  if (a < 10)       return "Safe";
  else if (a < 25)  return "Low";
  else if (a < 40)  return "Medium";
  else              return "High";
}

String classifyVibration(long delta) {
  if (delta < 100)       return "None";
  else if (delta < 500)  return "Low";
  else if (delta < 2000) return "Medium";
  else                   return "High";
}

// index = 0 for MPU1, 1 for MPU2
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

  String tiltLevel = classifyTilt(pitch);
  String vibLevel  = classifyVibration(delta);

  // Save current values
  prevAx[index] = ax;
  prevAy[index] = ay;
  prevAz[index] = az;
  firstRead[index] = false;

  // Build text line to send
  // Example: MPU1,12.3,Safe,350,Medium
  String line = "MPU";
  line += String(index + 1);
  line += ",";
  line += String(pitch, 1);
  line += ",";
  line += tiltLevel;
  line += ",";
  line += String(delta);
  line += ",";
  line += vibLevel;

  Serial.println(line);      // debug to PC
  espSerial.println(line);   // send to ESP
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

// ---------- Soil Moisture ----------
void processSoil() {
  int raw = analogRead(SOIL_PIN);

  int percent = map(raw, 1023, 300, 0, 100);  // adjust 300 if needed
  percent = constrain(percent, 0, 100);

  String level;
  if (percent < 20)      level = "Dry";
  else if (percent < 50) level = "Moist";
  else                   level = "Wet";

  // Example: SOIL,45,Moist
  String line = "SOIL,";
  line += String(percent);
  line += ",";
  line += level;

  Serial.println(line);
  espSerial.println(line);
}

// ---------- Rain Sensor ----------
void processRain() {
  int raw = analogRead(RAIN_PIN);

  int percent = map(raw, 1023, 300, 0, 100);  // adjust 300 if needed
  percent = constrain(percent, 0, 100);

  String level;
  if (percent < 20)      level = "NoRain";
  else if (percent < 60) level = "Light";
  else                   level = "Heavy";

  // Example: RAIN,30,Light
  String line = "RAIN,";
  line += String(percent);
  line += ",";
  line += level;

  Serial.println(line);
  espSerial.println(line);
}

void setup() {
  Serial.begin(9600);       // PC debug
  espSerial.begin(9600);    // must match ESP side
  Wire.begin();             // SDA=A4, SCL=A5 on UNO

  dht.begin();

  delay(1000);
  Serial.println("Arduino: sensors running...");
  Serial.println("Waking MPUs 0x68 and 0x69...");

  wakeMPU(MPU1_ADDR);
  wakeMPU(MPU2_ADDR);

  Serial.println("Setup done.\n");
}

void loop() {
  processMPU(0, MPU1_ADDR);  // MPU1 at 0x68
  delay(10);

  processMPU(1, MPU2_ADDR);  // MPU2 at 0x69
  delay(10);

  processDHT();              // every 2 seconds (internal timing)
  processSoil();
  delay(10);
  processRain();

  delay(300);
}
