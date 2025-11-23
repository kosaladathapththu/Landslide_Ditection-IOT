#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "DHT.h"

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

// -------- DHT22 ----------
#define DHTPIN D7
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// -------- Humidity Risk Thresholds (global, adjustable) ----------
float humidityLow = 20;      // %RH
float humidityModerate = 40; // %RH
float humidityHigh = 60;     // %RH

// -------- Functions ----------
String calculateHumidityRisk(float hum) {
  if (hum < humidityLow) return "Very Low";
  else if (hum < humidityModerate) return "Moderate";
  else if (hum < humidityHigh) return "High";
  else return "Critical";
}

void setup() {
  Serial.begin(9600);
  dht.begin();

  // Connect WiFi
  Serial.print("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");

  // Firebase setup
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = FIREBASE_EMAIL;
  auth.user.password = FIREBASE_PASSWORD;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Serial.println("Firebase Ready!");
}

void loop() {
  float hum = dht.readHumidity();
  float temp = dht.readTemperature(); // Celsius

  if (isnan(hum) || isnan(temp)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(2000);
    return;
  }

  String risk = calculateHumidityRisk(hum);

  Serial.printf("Humidity: %.2f %% | Temperature: %.2f C | Risk: %s\n", hum, temp, risk.c_str());

  // Upload to Firebase
  FirebaseJson json;
  json.set("humidity", hum);
  json.set("temperature", temp);
  json.set("risk", risk);

  if (Firebase.RTDB.setJSON(&fbdo, "/Landslide/Device1/Humidity", &json)) {
    Serial.println("Firebase Upload OK");
  } else {
    Serial.println("Firebase error: " + fbdo.errorReason());
  }

  delay(5000); // Update every 5 seconds
}
