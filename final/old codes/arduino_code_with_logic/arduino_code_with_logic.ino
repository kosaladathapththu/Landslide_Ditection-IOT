// ==========================================
//      ARDUINO UNO CODE (SENDER)
// ==========================================
// 1. Reads MPU1 (0x68) & MPU2 (0x69)
// 2. Reads DHT22, Soil, Rain
// 3. Sends all data to ESP8266 via SoftwareSerial (D2/D3)

#include <Wire.h>
#include <SoftwareSerial.h>
#include <math.h>
#include <DHT.h>

// --------- I2C Addresses (Physical MPUs) ----------
const uint8_t MPU1_ADDR = 0x68;  // Deep layer
const uint8_t MPU2_ADDR = 0x69;  // Middle layer

// --------- SoftwareSerial to ESP ----------
// Connect: Arduino D2 (RX) <--> ESP D6 (TX)
// Connect: Arduino D3 (TX) <--> ESP D5 (RX)
SoftwareSerial espSerial(2, 3);

// --------- Sensors ----------
#define DHTPIN 7
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define SOIL_PIN A0
#define RAIN_PIN A1

// --------- Constants ----------
const float gScale = 16384.0; 

// Variables for vibration calculation
int16_t prevAx[2] = {0, 0};
int16_t prevAy[2] = {0, 0};
int16_t prevAz[2] = {0, 0};
bool    firstRead[2] = {true, true};
float   DAE[2]       = {0, 0};

// Timers
unsigned long lastDhtMillis = 0;
const unsigned long DHT_INTERVAL = 2000;

// --- Helper: Wake MPU ---
void wakeMPU(uint8_t addr) {
  Wire.beginTransmission(addr);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();
  delay(50);
}

// --- Helper: Read Accel ---
bool readAccel(uint8_t addr, int16_t &ax, int16_t &ay, int16_t &az) {
  Wire.beginTransmission(addr);
  Wire.write(0x3B);
  if (Wire.endTransmission(false) != 0) return false;
  Wire.requestFrom(addr, (uint8_t)6);
  if (Wire.available() < 6) return false;
  ax = (Wire.read() << 8) | Wire.read();
  ay = (Wire.read() << 8) | Wire.read();
  az = (Wire.read() << 8) | Wire.read();
  return true;
}

// --- Helper: Compute Pitch ---
float computePitch(int16_t ax, int16_t ay, int16_t az) {
  float axg = (float)ax / gScale;
  float ayg = (float)ay / gScale;
  float azg = (float)az / gScale;
  return atan2(axg, sqrt(ayg * ayg + azg * azg)) * 180.0 / PI;
}

// --- Classification Logic ---
String classifyTiltLabel(float pitch) {
  float a = fabs(pitch);
  if (a < 8)        return "Stable";
  else if (a < 20)  return "MinorTilt";
  else if (a < 35)  return "MajorTilt";
  else              return "SlopeShift";
}

String classifyRiskCombined(float pitch, long dae, uint8_t depth) {
  float a = fabs(pitch);
  String base;
  if (a < 10 && dae < 150) base = "Safe";
  else if ((a < 20 && dae < 600) || dae < 400) base = "LowRisk";
  else if (a < 30 || dae < 1500) base = "Warning";
  else base = "HighRisk";

  if (depth == 0 && base == "HighRisk") return "DeepMovement";
  if (depth == 1 && base == "HighRisk") return "MidMovement";
  return base;
}

void processMPU(uint8_t index, uint8_t addr) {
  int16_t ax, ay, az;
  if (!readAccel(addr, ax, ay, az)) return;

  float pitch = computePitch(ax, ay, az);
  long delta = 0;
  if (!firstRead[index]) {
    delta = labs((long)ax - prevAx[index]) + labs((long)ay - prevAy[index]) + labs((long)az - prevAz[index]);
  }
  prevAx[index] = ax; prevAy[index] = ay; prevAz[index] = az;
  firstRead[index] = false;

  DAE[index] = (DAE[index] * 0.6f) + (delta * 0.4f);

  String tiltLevel = classifyTiltLabel(pitch);
  String vibLevel  = classifyRiskCombined(pitch, (long)DAE[index], index);

  // Send Line: MPU1,12.3,Stable,350,LowRisk
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

  Serial.println(line);
  espSerial.println(line);
}

void processOtherSensors() {
  // DHT (Runs every 2 seconds via DHT_INTERVAL)
  unsigned long now = millis();
  if (now - lastDhtMillis > DHT_INTERVAL) {
    lastDhtMillis = now;
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (!isnan(h) && !isnan(t)) {
      String line = "DHT," + String(t, 1) + "," + String(h, 1);
      Serial.println(line);
      espSerial.println(line);
    }
  }

  // --- SOIL & RAIN TIMER (2 SECONDS) ---
  static unsigned long lastSoilRain = 0;
  
  if (millis() - lastSoilRain > 2000) { 
    lastSoilRain = millis();
    
    // Soil
    int sRaw = analogRead(SOIL_PIN);
    int sPct = map(sRaw, 1023, 300, 0, 100); sPct = constrain(sPct, 0, 100);
    String sLvl = (sPct < 40) ? "Dry" : "Wet";
    String sLine = "SOIL," + String(sPct) + "," + sLvl;
    Serial.println(sLine);
    espSerial.println(sLine);

    // Rain
    int rRaw = analogRead(RAIN_PIN);
    int rPct = map(rRaw, 1023, 300, 0, 100); rPct = constrain(rPct, 0, 100);
    String rInt = (rPct < 10) ? "NoRain" : "Raining";
    String rLine = "RAIN," + String(rPct) + "," + rInt + ",0,Low";
    Serial.println(rLine);
    espSerial.println(rLine);
  }
}

void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);
  Wire.begin();
  dht.begin();
  delay(1000);
  
  // Wake physical MPUs
  wakeMPU(MPU1_ADDR);
  wakeMPU(MPU2_ADDR);
  
  Serial.println("Arduino: Sending MPU1 & MPU2...");
}

void loop() {
  processMPU(0, MPU1_ADDR); // Deep
  delay(20);
  processMPU(1, MPU2_ADDR); // Middle
  delay(20);
  processOtherSensors();
  delay(200);
}