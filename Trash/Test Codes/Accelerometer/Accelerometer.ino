#include <Wire.h>
#include <math.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>

// -------- WiFi ----------
#define WIFI_SSID "ZTE Blade V50 Design"
#define WIFI_PASSWORD "imaadh0234897651"

// -------- Firebase ----------
#define API_KEY "AIzaSyCSEWW0wawOzJgvxyOmDqXMqE8nihprCd0"
#define DATABASE_URL "https://iot-project-3c897-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_EMAIL "rushdeeimaadh@gmail.com"
#define FIREBASE_PASSWORD "ir@02348"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseJson json;

// -------- MPU6050 ----------
#define MPU1_ADDR 0x68
#define MPU2_ADDR 0x69
#define PWR_MGMT_1 0x6B
#define ACC_XOUT_H 0x3B

float baselinePitch1 = 0;
float baselinePitch2 = 0;

// -------- Global Thresholds ----------
float tiltVeryLow = 15;  // degrees
float tiltLowModerate = 25;
float tiltModerateHigh = 35;
float tiltHigh = 45;

float vibLowModerate = 0.2;  // g
float vibModerate = 0.3;
float vibHigh = 0.4;

// -------- MPU Helper Functions ----------
void initMPU(uint8_t addr) {
  Wire.beginTransmission(addr);
  Wire.write(PWR_MGMT_1);
  Wire.write(0);
  Wire.endTransmission();
  delay(10);
}

void readAccel(uint8_t addr, float &ax, float &ay, float &az) {
  Wire.beginTransmission(addr);
  Wire.write(ACC_XOUT_H);
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)addr, (size_t)6, true);

  if (Wire.available() < 6) {
    ax = ay = az = NAN;
    return;
  }

  int16_t rawX = (Wire.read() << 8) | Wire.read();
  int16_t rawY = (Wire.read() << 8) | Wire.read();
  int16_t rawZ = (Wire.read() << 8) | Wire.read();

  ax = rawX / 16384.0;
  ay = rawY / 16384.0;
  az = rawZ / 16384.0;
}

float getPitch(float ax, float ay, float az) {
  return atan2(ax, sqrt(ay * ay + az * az)) * (180.0 / PI);
}

float getVibrationMagnitude(float ax, float ay, float az) {
  return sqrt(ax * ax + ay * ay + az * az);
}

// -------- User Functions ----------
float getTiltingDifference(uint8_t sensorAddr, float &baselinePitch) {
  float ax, ay, az;
  readAccel(sensorAddr, ax, ay, az);
  float currentPitch = getPitch(ax, ay, az);
  return currentPitch - baselinePitch;
}

float getVibrationIntensity(uint8_t sensorAddr, int samples = 30, int delayMs = 10) {
  float ax, ay, az;
  float prevMag = 0;
  float totalChange = 0;
  for (int i = 0; i < samples; i++) {
    readAccel(sensorAddr, ax, ay, az);
    float mag = getVibrationMagnitude(ax, ay, az);
    totalChange += fabs(mag - prevMag);
    prevMag = mag;
    delay(delayMs);
  }
  return totalChange / samples;
}


String calculateTiltRisk(float tiltDiff) {
  if (tiltDiff < tiltVeryLow) return "Very Low";
  else if (tiltDiff < tiltLowModerate) return "Low-Moderate";
  else if (tiltDiff < tiltModerateHigh) return "Moderate-High";
  else if (tiltDiff < tiltHigh) return "High";
  else return "Highly Variable";
}

String calculateVibrationRisk(float vib) {
  if (vib > vibHigh) return "High";
  else if (vib > vibModerate) return "Moderate";
  else if (vib > vibLowModerate) return "Low-Moderate";
  else return "Very Low";
}


// -------- Setup ----------
void setup() {
  Serial.begin(9600);
  Wire.begin(D2, D1);  // SDA, SCL

  initMPU(MPU1_ADDR);
  initMPU(MPU2_ADDR);

  float ax, ay, az;
  readAccel(MPU1_ADDR, ax, ay, az);
  baselinePitch1 = getPitch(ax, ay, az);

  readAccel(MPU2_ADDR, ax, ay, az);
  baselinePitch2 = getPitch(ax, ay, az);

  Serial.print("Connecting WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("WiFi connected!");

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = FIREBASE_EMAIL;
  auth.user.password = FIREBASE_PASSWORD;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Serial.println("Firebase Ready!");
}

// -------- Loop ----------
void loop() {
  float tilt1 = getTiltingDifference(MPU1_ADDR, baselinePitch1);
  float tilt2 = getTiltingDifference(MPU2_ADDR, baselinePitch2);

  float vib1 = getVibrationIntensity(MPU1_ADDR);
  float vib2 = getVibrationIntensity(MPU2_ADDR);

  String tiltRisk1 = calculateTiltRisk(abs(tilt1));
  String vibRisk1 = calculateVibrationRisk(vib1);

  String tiltRisk2 = calculateTiltRisk(abs(tilt2));
  String vibRisk2 = calculateVibrationRisk(vib2);


  Serial.printf("Sensor1 -> Tilt: %.2f°, Vib: %.3f g, Risk: %s\n", tilt1, vib1, tiltRisk1.c_str());
  Serial.printf("Sensor2 -> Tilt: %.2f°, Vib: %.3f g, Risk: %s\n", tilt2, vib2, tiltRisk2.c_str());

  // Upload to Firebase
  json.clear();
  json.set("tilt1", tilt1);
  json.set("tiltRisk1", tiltRisk1);
  json.set("vibration1", vib1);
  json.set("vibRisk1", vibRisk1);

  json.set("tilt2", tilt2);
  json.set("tiltRisk2", tiltRisk2);
  json.set("vibration2", vib2);
  json.set("vibRisk2", vibRisk2);


  if (Firebase.RTDB.setJSON(&fbdo, "/Landslide/Device1/MPUData", &json))
    Serial.println("Firebase Upload OK");
  else
    Serial.println("Firebase error: " + fbdo.errorReason());

  delay(2000);
}
