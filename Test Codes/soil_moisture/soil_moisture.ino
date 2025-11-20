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

// -------- Soil Moisture ----------
#define SOIL_A_PIN A0  // analog pin for soil sensor

// -------- Risk thresholds ----------
int soilVeryLow = 20;   // %
int soilModerate = 40;  // %
int soilHigh = 60;      // %
int soilCritical = 61;  // anything above 60%

// -------- Functions ----------
int readSoilPercent() {
  int soilRaw = analogRead(SOIL_A_PIN);         // 0 = wet, 1023 = dry
  int soilPercent = map(soilRaw, 1023, 0, 0, 100); // Convert to 0-100%
  return soilPercent;
}

String calculateSoilRisk(int soilPercent) {
  if (soilPercent < soilVeryLow) return "Very Low Risk";
  else if (soilPercent < soilModerate) return "Moderate";
  else if (soilPercent < soilHigh) return "High";
  else return "Critical";
}

// -------- Setup ----------
void setup() {
  Serial.begin(9600);

  // WiFi
  Serial.print("Connecting WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) { delay(300); Serial.print("."); }
  Serial.println("WiFi connected!");

  // Firebase
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
  int soilPercent = readSoilPercent();
  String soilRisk = calculateSoilRisk(soilPercent);

  // Serial output
  Serial.printf("Soil Moisture: %d%%, Risk: %s\n", soilPercent, soilRisk.c_str());

  // Firebase upload
  json.clear();
  json.set("soilPercent", soilPercent);
  json.set("soilRisk", soilRisk);

  if (Firebase.RTDB.setJSON(&fbdo, "/Landslide/Device1/SoilData", &json))
    Serial.println("Firebase Upload OK");
  else
    Serial.println("Firebase error: " + fbdo.errorReason());

  delay(2000); // update every 2 seconds
}
