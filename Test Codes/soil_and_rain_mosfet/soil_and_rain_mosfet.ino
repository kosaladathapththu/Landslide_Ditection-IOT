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

// ---------- Sensors & MOSFET ----------
#define ANALOG_PIN A0

#define MOSFET_SOIL D5
#define MOSFET_RAIN D6

// ---------- Global thresholds ----------
int soilLow = 200;      // soil moisture low threshold (dry)
int soilModerate = 500; // moderate moisture
int soilHigh = 700;     // wet soil

int rainLow = 300;      // analog value for Low/Watch
int rainModerate = 600; // Moderate/Alert
int rainHigh = 900;     // High/Evacuation

// ---------- Functions ----------

// Read soil moisture through MOSFET
int readSoil() {
  digitalWrite(MOSFET_SOIL, HIGH);   // Power soil sensor
  delay(50);                         // allow sensor to stabilize
  int val = analogRead(ANALOG_PIN);
  digitalWrite(MOSFET_SOIL, LOW);    // Turn off sensor
  return val;
}

// Read rain sensor through MOSFET
int readRain() {
  digitalWrite(MOSFET_RAIN, HIGH);   // Power rain sensor
  delay(50);                         // allow sensor to stabilize
  int val = analogRead(ANALOG_PIN);
  digitalWrite(MOSFET_RAIN, LOW);    // Turn off sensor
  return val;
}

// Soil risk
String getSoilRisk(int val) {
  if (val < soilLow) return "Very Low Risk";
  else if (val < soilModerate) return "Moderate";
  else if (val < soilHigh) return "High";
  else return "Critical";
}

// Rain risk
String getRainRisk(int val) {
  if (val < rainLow) return "Low / Watch";
  else if (val < rainModerate) return "Moderate / Alert";
  else return "High / Evacuation";
}

// ---------- Setup ----------
void setup() {
  Serial.begin(9600);
  pinMode(MOSFET_SOIL, OUTPUT);
  pinMode(MOSFET_RAIN, OUTPUT);
  digitalWrite(MOSFET_SOIL, LOW);
  digitalWrite(MOSFET_RAIN, LOW);

  // Connect WiFi
  Serial.print("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) { delay(300); Serial.print("."); }
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
  int soilVal = readSoil();
  int rainVal = readRain();

  String soilRisk = getSoilRisk(soilVal);
  String rainRisk = getRainRisk(rainVal);

  // Print to Serial
  Serial.printf("Soil: %d, Risk: %s | Rain: %d, Risk: %s\n",
                soilVal, soilRisk.c_str(), rainVal, rainRisk.c_str());

  // Upload to Firebase
  Firebase.RTDB.setInt(&fbdo, "/Landslide/Device1/Soil/Value", soilVal);
  Firebase.RTDB.setString(&fbdo, "/Landslide/Device1/Soil/Risk", soilRisk);

  Firebase.RTDB.setInt(&fbdo, "/Landslide/Device1/Rain/Value", rainVal);
  Firebase.RTDB.setString(&fbdo, "/Landslide/Device1/Rain/Risk", rainRisk);

  delay(2000); // every 2 seconds
}
