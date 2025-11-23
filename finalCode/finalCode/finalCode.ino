#include <Wire.h>
#include <math.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

#define API_KEY ""
#define DATABASE_URL ""
#define FIREBASE_EMAIL ""
#define FIREBASE_PASSWORD ""

FirebaseData fbdo;
FirebaseData auth;
FirebaseData config;
FirebaseData json;

#define mpu1_address 0x68
#define mpu2_address 0x69
#define pwr_mgmt_1 0x6B
#define acc_xout_h 0x3B
#define soil_pin A0

float baselinePitch1 = 0;
float baselinePitch2 = 0;

float tiltVeryLow = 15;       
float tiltLowModerate = 25;
float tiltModerateHigh = 35;
float tiltHigh = 45;

float vibLowModerate = 0.2;   
float vibModerate = 0.3;
float vibHigh = 0.4;

int soilVeryLow = 20;   
int soilModerate = 40;  
int soilHigh = 60;      
int soilCritical = 61;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin(D2, D1);
  initMPU(mpu1_address);
  initMPU(mpu2_address);

  float ax, ay, az;
  readAccelerometer(mpu1_address, ax, ay, az);
  baselinePitch1 = getPitch(ax, ay, az);
  readAccelerometer(mpu2_address, ax, ay, az);
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

void loop() {
  // put your main code here, to run repeatedly:
  float tilt1 = getTiltingDifference(mpu1_address, baselinePitch1);
  float tilt2 = getTiltingDifference(mpu2_address, baselinePitch2);
  float vib1 = getVibrationIntensity(mpu1_address);
  float vib2 = getVibrationIntensity(mpu2_address);
  String tiltRisk1 = calculateTiltRisk(abs(tilt1));
  String vibRisk1  = calculateVibrationRisk(vib1);
  String tiltRisk2 = calculateTiltRisk(abs(tilt2));
  String vibRisk2 = calculateVibrationRisk(vib2);

  int soilPercent = readSoilPercent();
  String soilRisk = calculateSoilRisk(soilPercent);

  Serial.printf("Sensor1 -> Tilt: %.2f°, Vib: %.3f g, Risk: %s\n", tilt1, vib1, tiltRisk1.c_str());
  Serial.printf("Sensor2 -> Tilt: %.2f°, Vib: %.3f g, Risk: %s\n", tilt2, vib2, tiltRisk2.c_str());
  Serial.printf("Soil Moisture: %d%%, Risk: %s\n", soilPercent, soilRisk.c_str());

  json.clear();
  json.set("tilt1", tilt1);
  json.set("tiltRisk1", tiltRisk1);
  json.set("vibration1", vib1);
  json.set("vibRisk1", vibRisk1);
  json.set("tilt2", tilt2);
  json.set("tiltRisk2", tiltRisk2);
  json.set("vibration2", vib2);
  json.set("vibRisk2", vibRisk2);
  json.set("soilPercent", soilPercent);
  json.set("soilRisk", soilRisk);

  if (Firebase.RTDB.setJSON(&fbdo, "/Landslide/Device1/MPUData", &json))
    Serial.println("Firebase Upload OK");
  else
    Serial.println("Firebase error: " + fbdo.errorReason());

  delay(2000);

}

void initMPU(uint8_t addr) {
  Wire.beginTransmission(addr);
  Wire.write(pwr_mgmt_1);
  Wire.write(0);
  Wire.endTransmission();
  delay(10);
}

void readAccelerometer(uint8_t addr, float &ax, float &ay, float &az) {
  Wire.beginTransmission(addr);
  Wire.write(acc_xout_h);
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)addr, (size_t)6, true);

  if (Wire.available() < 6) { ax = ay = az = NAN; return; }

  int16_t rawX = (Wire.read() << 8) | Wire.read();
  int16_t rawY = (Wire.read() << 8) | Wire.read();
  int16_t rawZ = (Wire.read() << 8) | Wire.read();

  ax = rawX / 16384.0;
  ay = rawY / 16384.0;
  az = rawZ / 16384.0;
}

float getPitch(float ax, float ay, float az) {
  return atan2(ax, sqrt(ay*ay + az*az)) * (180.0/PI);
}

float getVibrationMagnitude(float ax, float ay, float az) {
  return sqrt(ax*ax + ay*ay + az*az);
}

float getTiltingDifference(uint8_t sensorAddr, float &baselinePitch) {
  float ax, ay, az;
  readAccelerometer(sensorAddr, ax, ay, az);
  float currentPitch = getPitch(ax, ay, az);
  return currentPitch - baselinePitch;
}

float getVibrationIntensity(uint8_t sensorAddr, int samples = 30, int delayMs = 10) {
  float ax, ay, az;
  float prevMag = 0;
  float totalChange = 0;
  for (int i = 0; i < samples; i++) {
    readAccelerometer(sensorAddr, ax, ay, az);
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

int readSoilPercent() {
  int soilRaw = analogRead(soil_pin);         
  int soilPercent = map(soilRaw, 1023, 0, 0, 100); 
  return soilPercent;
}

String calculateSoilRisk(int soilPercent) {
  if (soilPercent < soilVeryLow) return "Very Low Risk";
  else if (soilPercent < soilModerate) return "Moderate";
  else if (soilPercent < soilHigh) return "High";
  else return "Critical";
}