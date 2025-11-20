#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>

// ---------- WiFi ----------
#define WIFI_SSID "ZTE Blade V50 Design"
#define WIFI_PASSWORD "imaadh0234897651"

// ---------- Firebase ----------
#define API_KEY "AIzaSyCSEWW0wawOzJgvxyOmDqXMqE8nihprCd0"
#define DATABASE_URL "https://iot-project-3c897-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_EMAIL "rushdeeimaadh@gmail.com"
#define FIREBASE_PASSWORD "ir@02348"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// ---------- Rain Sensor ----------
#define RAIN_PIN A0           // Analog pin for sensor

// ---------- Global thresholds (tuneable) ----------
int rainLow = 300;           // analog value threshold for Low/Watch
int rainModerate = 600;      // analog value threshold for Moderate/Alert
int rainHigh = 900;          // analog value threshold for High/Evacuation

// ---------- Function to read rain sensor ----------
int readRainAnalog() {
  int value = analogRead(RAIN_PIN);
  return value;
}

// ---------- Function to get rain risk ----------
String getRainRisk(int rainValue) {
  if (rainValue < rainLow) return "Low / Watch";
  else if (rainValue < rainModerate) return "Moderate / Alert";
  else return "High / Evacuation";
}

// ---------- Setup ----------
void setup() {
  Serial.begin(9600);
  pinMode(RAIN_PIN, INPUT);

  // Connect WiFi
  Serial.print("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("WiFi connected!");

  // Setup Firebase
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = FIREBASE_EMAIL;
  auth.user.password = FIREBASE_PASSWORD;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Serial.println("Firebase Ready!");
}

// ---------- Loop ----------
void loop() {
  int rainValue = readRainAnalog();
  String rainRisk = getRainRisk(rainValue);

  // Print to Serial
  Serial.printf("Rain Analog: %d, Risk: %s\n", rainValue, rainRisk.c_str());

  // Upload to Firebase
  if (Firebase.RTDB.setString(&fbdo, "/Landslide/Device1/RainSensor/Risk", rainRisk))
    Serial.println("Firebase Upload OK");
  else
    Serial.println("Firebase error: " + fbdo.errorReason());

  if (Firebase.RTDB.setInt(&fbdo, "/Landslide/Device1/RainSensor/Value", rainValue))
    Serial.println("Firebase Upload OK");
  else
    Serial.println("Firebase error: " + fbdo.errorReason());

  delay(2000); // update every 2 seconds
}
