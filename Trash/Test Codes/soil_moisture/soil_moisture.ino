#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

// ---------------- WiFi ----------------
#define WIFI_SSID "ZTE Blade V50 Design"
#define WIFI_PASSWORD "imaadh0234897651"

// ---------------- Firebase -------------
#define API_KEY "AIzaSyCSEWW0wawOzJgvxyOmDqXMqE8nihprCd0"
#define DATABASE_URL "https://iot-project-3c897-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_EMAIL "rushdeeimaadh@gmail.com"
#define FIREBASE_PASSWORD "ir@02348"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseJson json;

// ---------------- Arduino Serial to ESP -----
SoftwareSerial arduinoSerial(D7, D8); 
// RX = D7 (GPIO13), TX = D8 (GPIO15)

String riskLevel(int v) {
  if (v < 20) return "Very Low Risk";
  else if (v < 40) return "Moderate";
  else if (v < 60) return "High";
  else return "Critical";
}

void setup() {
  Serial.begin(9600);           // Monitor
  arduinoSerial.begin(9600);    // Arduino communication

  Serial.println("Connecting WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) { delay(300); Serial.print("."); }
  Serial.println("\nWiFi Connected!");

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = FIREBASE_EMAIL;
  auth.user.password = FIREBASE_PASSWORD;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("Firebase Ready!");
}

void loop() {
  if (arduinoSerial.available()) {
    int soil = arduinoSerial.parseInt();

    if (soil > 0 && soil <= 100) {
      String risk = riskLevel(soil);

      Serial.printf("Soil: %d%%  Risk: %s\n", soil, risk.c_str());

      json.clear();
      json.set("soilPercent", soil);
      json.set("soilRisk", risk);

      if (Firebase.RTDB.setJSON(&fbdo, "/Landslide/Device1/SoilData", &json))
        Serial.println("Uploaded to Firebase!");
      else
        Serial.println("Firebase Error: " + fbdo.errorReason());
    }
  }
}
