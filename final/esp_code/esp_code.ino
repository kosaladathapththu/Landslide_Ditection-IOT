#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <FirebaseJson.h>
#include <SoftwareSerial.h>

#define WIFI_SSID     "Kosala's Galaxy A21s"
#define WIFI_PASSWORD "11111111"

#define FIREBASE_URL  "https://iot-project-3c897-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define FIREBASE_KEY  "AIzaSyCSEWW0wawOzJgvxyOmDqXMqE8nihprCd0"

#define SERVO1_PIN D1
#define SERVO2_PIN D2

SoftwareSerial arduinoSerial(D5, D6); // RX,TX to Arduino
FirebaseData fbdo;
FirebaseJson json;

// ------------- RISK HELPERS (same style as UNO) -------------
String classifyTilt(float angle) {
  float a = fabs(angle);
  if (a < 15)        return "Very Low";
  else if (a < 25)   return "Low-Moderate";
  else if (a < 35)   return "Moderate-High";
  else if (a < 45)   return "High";
  else               return "Critical";
}

String classifySoil(float soilPercent, float mainTilt) {
  float a = fabs(mainTilt);
  if (soilPercent < 20)                      return "Very Low";
  else if (soilPercent < 40 && a > 25)      return "Moderate";
  else if (soilPercent < 60 && a > 25)      return "High";
  else if (soilPercent > 60 && a > 15)      return "Critical";
  else                                      return "Very Low / Safe";
}

String classifyRain(float rainMM) {
  if (rainMM >= 150)      return "High / Evacuation";
  else if (rainMM >= 100) return "Moderate";
  else if (rainMM >= 75)  return "Low";
  else                    return "Very Low / Safe";
}

String classifyHumidity(float hum) {
  if (hum >= 90)      return "Critical";
  else if (hum >= 80) return "High";
  else if (hum >= 60) return "Moderate";
  else                return "Very Low / Safe";
}

String classifyVibration(float vib) {
  float v = fabs(vib);
  if (v < 0.4)       return "Very Low";
  else if (v < 0.8)  return "Medium";
  else               return "High";
}

// ------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  arduinoSerial.begin(9600);

  pinMode(SERVO1_PIN, OUTPUT);
  pinMode(SERVO2_PIN, OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
  }

  // old-style init (OK with Firebase_ESP_Client examples)
  Firebase.begin(FIREBASE_URL, FIREBASE_KEY);
  Firebase.reconnectWiFi(true);
}

void loop() {

  // ================== READ CSV FROM ARDUINO ==================
  if (arduinoSerial.available()) {
    String data = arduinoSerial.readStringUntil('\n');
    data.trim();
    if (data.length() == 0) return;

    float values[10];
    int start = 0, idx = 0;
    while (idx < 10) {
      int comma = data.indexOf(',', start);
      if (comma == -1) comma = data.length();
      values[idx++] = data.substring(start, comma).toFloat();
      start = comma + 1;
    }

    float tilt1 = values[0];
    float tilt2 = values[1];
    float tilt3 = values[2];
    float vib1  = values[3];
    float vib2  = values[4];
    float vib3  = values[5];
    float soil  = values[6];
    float rain  = values[7];
    float hum   = values[8];
    float temp  = values[9];

    // ---------- Calculate risks (for new DB) ----------
    String tiltRisk1 = classifyTilt(tilt1);
    String tiltRisk2 = classifyTilt(tilt2);   // you can also use tilt3 if needed
    String soilRisk  = classifySoil(soil, tilt1);
    String rainRisk  = classifyRain(rain);
    String humRisk   = classifyHumidity(hum);
    String vibRisk1  = classifyVibration(vib1);
    String vibRisk2  = classifyVibration(vib2);

    // ================== WRITE TO NEW DATABASE ==================

    // 🌱 SoilData
    FirebaseJson soilJson;
    soilJson.set("soilPercent", soil);
    soilJson.set("soilRisk", soilRisk);
    Firebase.RTDB.setJSON(&fbdo, "/SoilData", &soilJson);

    // 🌧 RainSensor
    FirebaseJson rainJson;
    rainJson.set("rainMM", rain);
    rainJson.set("risk", rainRisk);
    Firebase.RTDB.setJSON(&fbdo, "/RainSensor", &rainJson);

    // 💧 Humidity
    FirebaseJson humJson;
    humJson.set("humidity", hum);
    humJson.set("temperature", temp);
    humJson.set("risk", humRisk);
    Firebase.RTDB.setJSON(&fbdo, "/Humidity", &humJson);

    // 📈 MPUData (tilt + vibration + risks)
    FirebaseJson mpuJson;
    mpuJson.set("tilt1", tilt1);
    mpuJson.set("tilt2", tilt2);
    mpuJson.set("tilt3", tilt3);
    mpuJson.set("vibration1", vib1);
    mpuJson.set("vibration2", vib2);
    mpuJson.set("vibration3", vib3);
    mpuJson.set("tiltRisk1", tiltRisk1);
    mpuJson.set("tiltRisk2", tiltRisk2);
    mpuJson.set("vibRisk1", vibRisk1);
    mpuJson.set("vibRisk2", vibRisk2);
    Firebase.RTDB.setJSON(&fbdo, "/MPUData", &mpuJson);

    // (optional) pitches under /accelerometers for backup
    FirebaseJson accJson;
    accJson.set("pitch1", tilt1);
    accJson.set("pitch2", tilt2);
    accJson.set("pitch3", tilt3);
    Firebase.RTDB.setJSON(&fbdo, "/accelerometers", &accJson);
  }

  // ================== SERVO / BARRIER CONTROL ==================
  // Dashboard writes: /Controls/barrier  (true = CLOSE, false = OPEN)
  if (Firebase.RTDB.getBool(&fbdo, "/Controls/barrier")) {
    bool barrierClosed = fbdo.boolData();

    if (barrierClosed) {
      // CLOSE = same as your old "CLOSE" command
      digitalWrite(SERVO1_PIN, HIGH);
      digitalWrite(SERVO2_PIN, HIGH);
    } else {
      // OPEN
      digitalWrite(SERVO1_PIN, LOW);
      digitalWrite(SERVO2_PIN, LOW);
    }
  }

  delay(1000);
}
