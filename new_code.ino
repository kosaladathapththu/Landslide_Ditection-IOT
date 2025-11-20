#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include "DHT.h"
#include <Servo.h>
#include <math.h>

#define WIFI_SSID     "ZTE Blade V50 Design"
#define WIFI_PASSWORD "imaadh0234897651"

#define API_KEY      "AIzaSyCrUFSVab6xJkxE95LjMUm9w_xiq2m89L0"
#define DATABASE_URL "https://landslide-detection-50870-default-rtdb.asia-southeast1.firebasedatabase.app/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseJson json;

#define DHTPIN   D5
#define DHTTYPE  DHT22
DHT dht(DHTPIN, DHTTYPE);

#define SOIL_PIN D6
#define RAIN_PIN A0

#define GREEN_LED  D0
#define YELLOW_LED D4
#define RED_LED    D3
#define BUZZER     D8

Servo servo1;
Servo servo2;

// MPU6050 addresses
#define MPU1 0x68
#define MPU2 0x69

#define PWR_MGMT_1 0x6B
#define ACC_XOUT_H 0x3B

float pitch1 = 0;
float pitch2 = 0;

void initMPU(int addr) {
  Wire.beginTransmission(addr);
  Wire.write(PWR_MGMT_1);
  Wire.write(0);
  Wire.endTransmission();
}

float readPitch(int address) {
  Wire.beginTransmission(address);
  Wire.write(ACC_XOUT_H);
  Wire.endTransmission(false);

  Wire.requestFrom((uint8_t)address, (size_t)6, (bool)true);

  int16_t ax = (Wire.read() << 8) | Wire.read();
  int16_t ay = (Wire.read() << 8) | Wire.read();
  int16_t az = (Wire.read() << 8) | Wire.read();

  float axg = ax / 16384.0;
  float ayg = ay / 16384.0;
  float azg = az / 16384.0;

  float pitch = atan2(axg, sqrt(ayg * ayg + azg * azg)) * (180.0 / 3.14159);
  return pitch;
}

// -----------------------------
// WARNING SYSTEM
// -----------------------------
void setWarning(String level) {
  if (level == "LOW") {
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, LOW);
    digitalWrite(BUZZER, LOW);

    servo1.write(0);
    servo2.write(0);
  }
  else if (level == "MODERATE") {
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, HIGH);
    digitalWrite(RED_LED, LOW);
    digitalWrite(BUZZER, LOW);

    servo1.write(30);
    servo2.write(30);
  }
  else if (level == "HIGH") {
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, HIGH);
    digitalWrite(BUZZER, HIGH);

    servo1.write(90);
    servo2.write(90);
  }
}

// -----------------------------
// FIREBASE TEST
// -----------------------------
void testFirebase() {
  Serial.println("Testing Firebase...");
  if (Firebase.RTDB.setString(&fbdo, "/test", "OK")) {
    Serial.println("Firebase test success!");
  } else {
    Serial.print("Firebase test failed: ");
    Serial.println(fbdo.errorReason());
  }
}

// -----------------------------
// SETUP
// -----------------------------
void setup() {
  Serial.begin(9600);
  delay(500);

  Wire.begin(D2, D1);  // SDA=D2, SCL=D1

  initMPU(MPU1);
  initMPU(MPU2);

  dht.begin();

  pinMode(SOIL_PIN, INPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  servo1.attach(D0);
  servo2.attach(D7);   // <-- Your selection (SAFE)

  // ---- WiFi ----
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // ---- Firebase ----
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  auth.user.email    = "kosalaathapaththu1234@gmail.com";
  auth.user.password = "Plapytome2@";

  Firebase.reconnectWiFi(true);
  Firebase.begin(&config, &auth);

  delay(2000);
  testFirebase();
}

// -----------------------------
// LOOP
// -----------------------------
void loop() {
  pitch1 = readPitch(MPU1);
  pitch2 = readPitch(MPU2);

  int soil = digitalRead(SOIL_PIN);
  int rainRaw = analogRead(RAIN_PIN);
  int rainMM = map(rainRaw, 1023, 0, 0, 200);

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    humidity = -1;
    temperature = -1;
  }

  String level = "LOW";
  bool tiltMajor = abs(pitch1) > 12 || abs(pitch2) > 12;

  if (tiltMajor && rainMM >= 150) level = "HIGH";
  else if (rainMM >= 100 || soil == 0 || abs(pitch1) > 7 || abs(pitch2) > 7)
    level = "MODERATE";

  setWarning(level);

  // ---- Serial Output ----
  Serial.println("----- SENSOR DATA -----");
  Serial.print("Pitch1: "); Serial.println(pitch1);
  Serial.print("Pitch2: "); Serial.println(pitch2);
  Serial.print("Soil (0=wet): "); Serial.println(soil);
  Serial.print("Rain Raw: "); Serial.println(rainRaw);
  Serial.print("Rain MM: "); Serial.println(rainMM);
  Serial.print("Temp: "); Serial.println(temperature);
  Serial.print("Humidity: "); Serial.println(humidity);
  Serial.print("Level: "); Serial.println(level);
  Serial.println("-----------------------\n");

  // ---- Firebase Upload ----
  json.clear();
  json.set("pitch1", pitch1);
  json.set("pitch2", pitch2);
  json.set("soil", soil);
  json.set("rainRaw", rainRaw);
  json.set("rainMM", rainMM);
  json.set("temperature", temperature);
  json.set("humidity", humidity);
  json.set("level", level);

  if (Firebase.RTDB.setJSON(&fbdo, "/landslideSystem", &json)) {
    Serial.println("Firebase updated!\n");
  } else {
    Serial.print("Firebase error: ");
    Serial.println(fbdo.errorReason());
  }

  delay(1000);
}