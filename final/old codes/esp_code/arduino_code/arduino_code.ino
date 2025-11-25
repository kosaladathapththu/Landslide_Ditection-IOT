

#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <SoftwareSerial.h>

#define WIFI_SSID     "ZTE Blade V50 Design"
#define WIFI_PASSWORD "imaadh0234897651"

// -------- Firebase --------
#define API_KEY      "AIzaSyCSEWW0wawOzJgvxyOmDqXMqE8nihprCd0"
#define DATABASE_URL "https://iot-project-3c897-default-rtdb.asia-southeast1.firebasedatabase.app"

#define USER_EMAIL    "kosalaathapaththu1234@gmail.com"
#define USER_PASSWORD "Plapytome2@"

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Arduino → ESP serial pins
// Arduino D3 (TX) → ESP D5 (RX)
// Arduino D2 (RX) → ESP D6 (TX)
SoftwareSerial arduinoSerial(D5, D6);

// ----------- Firebase error print -----------
void printFbError(const String &tag) {
  Serial.print("[");
  Serial.print(tag);
  Serial.print("] Error: ");
  Serial.println(fbdo.errorReason());
}

// ----------- Upload MPU values -----------
void sendMPU(const String &label, float pitch, const String &tilt, long delta, const String &vib) {
  String base = "/sensors/" + label;  // /sensors/MPU1, /sensors/MPU2, /sensors/MPU3

  if (!Firebase.RTDB.setFloat(&fbdo, base + "/pitch", pitch))
    printFbError(label + " pitch");

  if (!Firebase.RTDB.setString(&fbdo, base + "/tilt", tilt))
    printFbError(label + " tilt");

  if (!Firebase.RTDB.setInt(&fbdo, base + "/delta", delta))
    printFbError(label + " delta");

  if (!Firebase.RTDB.setString(&fbdo, base + "/vib", vib))
    printFbError(label + " vib");
}

// ----------- Upload DHT22 -----------
void sendDHT(float temp, float hum) {
  if (!Firebase.RTDB.setFloat(&fbdo, "/sensors/DHT/temperature", temp))
    printFbError("DHT temp");

  if (!Firebase.RTDB.setFloat(&fbdo, "/sensors/DHT/humidity", hum))
    printFbError("DHT hum");
}

// ----------- Upload Soil -----------
void sendSoil(int val, const String &level) {
  if (!Firebase.RTDB.setInt(&fbdo, "/sensors/soil/value", val))
    printFbError("Soil value");

  if (!Firebase.RTDB.setString(&fbdo, "/sensors/soil/level", level))
    printFbError("Soil level");
}

// ----------- Upload Rain (extended) -----------
// RAIN,<percent>,<intensity>,<mmLastHour>,<risk>
void sendRain(int val, const String &intensity, float mmLastHour, const String &risk) {
  String base = "/sensors/rain";

  if (!Firebase.RTDB.setInt(&fbdo, base + "/value", val))
    printFbError("Rain value");

  if (!Firebase.RTDB.setString(&fbdo, base + "/intensity", intensity))
    printFbError("Rain intensity");

  if (!Firebase.RTDB.setFloat(&fbdo, base + "/mmLastHour", mmLastHour))
    printFbError("Rain mmLastHour");

  if (!Firebase.RTDB.setString(&fbdo, base + "/risk", risk))
    printFbError("Rain risk");
}

void setup() {
  Serial.begin(9600);
  arduinoSerial.begin(9600);

  Serial.println();
  Serial.println("ESP8266: WiFi + Firebase + Serial Reader (LAB)");

  // -------- WiFi connect --------
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("WiFi OK. IP: ");
  Serial.println(WiFi.localIP());

  // -------- Firebase Setup --------
  Serial.println("Configuring Firebase...");
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Serial.println("Firebase.begin() done ✅");

  // -------- Test write to DB --------
  Serial.println("Testing write to Firebase at /test/path ...");
  if (Firebase.RTDB.setString(&fbdo, "/test/path", "hello from ESP8266")) {
    Serial.println("Test write OK ✅");
  } else {
    printFbError("Test write");
  }

  Serial.println("Firebase Initialized! Waiting for Arduino...");
}

void loop() {

  if (!arduinoSerial.available()) {
    return;
  }

  String line = arduinoSerial.readStringUntil('\n');
  line.trim();
  if (line.length() == 0) return;

  Serial.print("RAW: ");
  Serial.println(line);

  // ----- MPU1 / MPU2 / MPU3 -----
  if (line.startsWith("MPU")) {

    int c1 = line.indexOf(',');
    int c2 = line.indexOf(',', c1 + 1);
    int c3 = line.indexOf(',', c2 + 1);
    int c4 = line.indexOf(',', c3 + 1);

    if (c1 < 0 || c2 < 0 || c3 < 0 || c4 < 0) {
      Serial.println("MPU line parse error");
      return;
    }

    String label = line.substring(0, c1);             // MPU1 / MPU2 / MPU3
    float pitch  = line.substring(c1 + 1, c2).toFloat();
    String tilt  = line.substring(c2 + 1, c3);
    long delta   = line.substring(c3 + 1, c4).toInt();
    String vib   = line.substring(c4 + 1);

    Serial.print(label);
    Serial.print(" Parsed -> pitch=");
    Serial.print(pitch);
    Serial.print(", tilt=");
    Serial.print(tilt);
    Serial.print(", delta=");
    Serial.print(delta);
    Serial.print(", vib=");
    Serial.println(vib);

    if (Firebase.ready()) {
      sendMPU(label, pitch, tilt, delta, vib);
    } else {
      Serial.println("Firebase not ready, MPU not sent");
    }
  }

  // ----- DHT -----
  else if (line.startsWith("DHT")) {

    int c1 = line.indexOf(',');
    int c2 = line.indexOf(',', c1 + 1);

    if (c1 < 0 || c2 < 0) {
      Serial.println("DHT line parse error");
      return;
    }

    float t = line.substring(c1 + 1, c2).toFloat();
    float h = line.substring(c2 + 1).toFloat();

    Serial.print("DHT Parsed -> T=");
    Serial.print(t);
    Serial.print("C, H=");
    Serial.print(h);
    Serial.println("%");

    if (Firebase.ready()) {
      sendDHT(t, h);
    } else {
      Serial.println("Firebase not ready, DHT not sent");
    }
  }

  // ----- SOIL -----
  else if (line.startsWith("SOIL")) {

    int c1 = line.indexOf(',');
    int c2 = line.indexOf(',', c1 + 1);

    if (c1 < 0 || c2 < 0) {
      Serial.println("SOIL line parse error");
      return;
    }

    int val = line.substring(c1 + 1, c2).toInt();
    String level = line.substring(c2 + 1);

    Serial.print("SOIL Parsed -> ");
    Serial.print(val);
    Serial.print("%, level=");
    Serial.println(level);

    if (Firebase.ready()) {
      sendSoil(val, level);
    } else {
      Serial.println("Firebase not ready, SOIL not sent");
    }
  }

  // ----- RAIN (extended) -----
  // Format: RAIN,<percent>,<intensity>,<mmLastHour>,<risk>
  else if (line.startsWith("RAIN")) {

    int c1 = line.indexOf(',');
    int c2 = line.indexOf(',', c1 + 1);
    int c3 = line.indexOf(',', c2 + 1);
    int c4 = line.indexOf(',', c3 + 1);

    if (c1 < 0 || c2 < 0 || c3 < 0 || c4 < 0) {
      Serial.println("RAIN line parse error");
      return;
    }

    int val = line.substring(c1 + 1, c2).toInt();
    String intensity = line.substring(c2 + 1, c3);
    float mmLastHour = line.substring(c3 + 1, c4).toFloat();
    String risk = line.substring(c4 + 1);

    Serial.print("RAIN Parsed -> ");
    Serial.print(val);
    Serial.print("%, intensity=");
    Serial.print(intensity);
    Serial.print(", mmLastHour=");
    Serial.print(mmLastHour);
    Serial.print(", risk=");
    Serial.println(risk);

    if (Firebase.ready()) {
      sendRain(val, intensity, mmLastHour, risk);
    } else {
      Serial.println("Firebase not ready, RAIN not sent");
    }
  }

  else {
    Serial.println("Unknown line");
  }
}