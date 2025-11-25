#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <FirebaseJson.h>
#include <SoftwareSerial.h>
#include <math.h>

// -------- WiFi --------
#define WIFI_SSID     "Kosala's Galaxy A21s"
#define WIFI_PASSWORD "11111111"

// -------- Firebase --------
#define FIREBASE_URL  "https://iot-project-3c897-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define FIREBASE_KEY  "AIzaSyCSEWW0wawOzJgvxyOmDqXMqE8nihprCd0"

// -------- Servo / Barrier (to road gate) --------
#define SERVO1_PIN D1
#define SERVO2_PIN D2

// UNO <-> ESP serial
SoftwareSerial arduinoSerial(D5, D6); // RX, TX

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// ------------- RISK HELPERS -------------
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

// (if you still need a rain classifier when no level string comes)
String classifyRain(float rainMM) {
  if (rainMM >= 150)      return "High / Evacuation";
  else if (rainMM >= 100) return "Moderate";
  else if (rainMM >= 75)  return "Low";
  else                    return "Very Low / Safe";
}
// ----------------------------------------

void setup() {
  Serial.begin(115200);
  arduinoSerial.begin(9600);

  pinMode(SERVO1_PIN, OUTPUT);
  pinMode(SERVO2_PIN, OUTPUT);

  // ----- WiFi -----
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi Connected, IP: ");
  Serial.println(WiFi.localIP());

  // ----- Firebase (Firebase_ESP_Client style) -----
  config.api_key = FIREBASE_KEY;
  config.database_url = FIREBASE_URL;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("Firebase initialized");
}

void loop() {

  // ================== READ FROM ARDUINO ==================
  if (arduinoSerial.available()) {
    String line = arduinoSerial.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) return;

    Serial.print("Received line: ");
    Serial.println(line);

    // ------------------------------------------------
    // 1. RAIN LINE
    //    Format: RAIN,wet=45,level=MEDIUM,nowMin=3.5,index=1.20
    // ------------------------------------------------
    if (line.startsWith("RAIN")) {
      float wetValue      = 0;
      String levelValue   = "";
      float nowMinValue   = 0;
      float indexValue    = 0;

      // wet=
      int wIndex = line.indexOf("wet=");
      if (wIndex != -1) {
        int commaIndex = line.indexOf(",", wIndex);
        String wStr;
        if (commaIndex == -1) {
          wStr = line.substring(wIndex + 4);
        } else {
          wStr = line.substring(wIndex + 4, commaIndex);
        }
        wetValue = wStr.toFloat();
      }

      // level=
      int levelIndex = line.indexOf("level=");
      if (levelIndex != -1) {
        int commaIndex = line.indexOf(",", levelIndex);
        if (commaIndex == -1) {
          levelValue = line.substring(levelIndex + 6);
        } else {
          levelValue = line.substring(levelIndex + 6, commaIndex);
        }
      }

      // nowMin=
      int nowIndex = line.indexOf("nowMin=");
      if (nowIndex != -1) {
        int commaIndex = line.indexOf(",", nowIndex);
        String nStr;
        if (commaIndex == -1) {
          nStr = line.substring(nowIndex + 7);
        } else {
          nStr = line.substring(nowIndex + 7, commaIndex);
        }
        nowMinValue = nStr.toFloat();
      }

      // index=
      int idxIndex = line.indexOf("index=");
      if (idxIndex != -1) {
        String iStr = line.substring(idxIndex + 6);
        indexValue = iStr.toFloat();
      }

      Serial.print("RAIN Parsed → wet=");
      Serial.print(wetValue);
      Serial.print(" | level=");
      Serial.print(levelValue);
      Serial.print(" | nowMin=");
      Serial.print(nowMinValue);
      Serial.print(" | index=");
      Serial.println(indexValue);

      // 🔥 Upload to NEW RainSensor node
      //   wetPercent   – raw wet value from Arduino
      //   risk         – level string from Arduino
      //   wetSeconds   – time sensor was wet
      //   rainMM       – index or mm value
      Firebase.RTDB.setFloat (&fbdo, "/RainSensor/wetPercent",  wetValue);
      Firebase.RTDB.setString(&fbdo, "/RainSensor/risk",        levelValue);
      Firebase.RTDB.setFloat (&fbdo, "/RainSensor/wetSeconds",  nowMinValue * 60.0);
      Firebase.RTDB.setFloat (&fbdo, "/RainSensor/rainMM",      indexValue);

      return; // done with RAIN line
    }

    // ------------------------------------------------
    // 2. DEFAULT CSV LINE
    //    Example: 10 floats
    //    tilt1,tilt2,tilt3,vib1,vib2,vib3,soil,rain,hum,temp
    //    (if you don't send rain in CSV, it will just be 0)
    // ------------------------------------------------
    float values[10];
    int start = 0, idx = 0;

    while (idx < 10) {
      int comma = line.indexOf(',', start);
      if (comma == -1) comma = line.length();
      String part = line.substring(start, comma);
      values[idx++] = part.toFloat();
      start = comma + 1;
      if (start >= (int)line.length()) break;
    }

    // only proceed if we got at least 9-10 numbers
    if (idx >= 9) {
      float tilt1 = values[0];
      float tilt2 = values[1];
      float tilt3 = values[2];
      float vib1  = values[3];
      float vib2  = values[4];
      float vib3  = values[5];
      float soil  = values[6];
      // float rainCSV = values[7]; // optional, if you still send it
      float hum   = values[8];
      float temp  = (idx > 9 ? values[9] : 0);

      Serial.println("CSV Parsed →");
      Serial.print(" tilt1="); Serial.print(tilt1);
      Serial.print(" tilt2="); Serial.print(tilt2);
      Serial.print(" tilt3="); Serial.print(tilt3);
      Serial.print(" vib1=");  Serial.print(vib1);
      Serial.print(" vib2=");  Serial.print(vib2);
      Serial.print(" vib3=");  Serial.print(vib3);
      Serial.print(" soil=");  Serial.print(soil);
      Serial.print(" hum=");   Serial.print(hum);
      Serial.print(" temp=");  Serial.println(temp);

      // ---------- Calculate risks ----------
      String tiltRisk1 = classifyTilt(tilt1);
      String tiltRisk2 = classifyTilt(tilt2);   // you can also use tilt3
      String soilRisk  = classifySoil(soil, tilt1);
      String humRisk   = classifyHumidity(hum);
      String vibRisk1  = classifyVibration(vib1);
      String vibRisk2  = classifyVibration(vib2);

      // ---------------- SoilData ----------------
      {
        FirebaseJson soilJson;
        soilJson.set("soilPercent", soil);
        soilJson.set("soilRisk", soilRisk);
        Firebase.RTDB.setJSON(&fbdo, "/SoilData", &soilJson);
      }

      // ---------------- Humidity ----------------
      {
        FirebaseJson humJson;
        humJson.set("humidity", hum);
        humJson.set("temperature", temp);
        humJson.set("risk", humRisk);
        Firebase.RTDB.setJSON(&fbdo, "/Humidity", &humJson);
      }

      // ---------------- MPUData -----------------
      {
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
      }

      // ------------- accelerometers (angles only) -------------
      {
        FirebaseJson accJson;
        accJson.set("pitch1", tilt1);
        accJson.set("pitch2", tilt2);
        accJson.set("pitch3", tilt3);
        Firebase.RTDB.setJSON(&fbdo, "/accelerometers", &accJson);
      }

      // (RainSensor is updated by RAIN line, so we don't touch it here)
    }

  } // end if arduinoSerial.available()

  // ================== SERVO / BARRIER CONTROL ==================
  // Dashboard writes: /Controls/barrier  (true = CLOSE, false = OPEN)
  if (Firebase.RTDB.getBool(&fbdo, "/Controls/barrier")) {
    bool barrierClosed = fbdo.boolData();

    if (barrierClosed) {
      // CLOSE
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
