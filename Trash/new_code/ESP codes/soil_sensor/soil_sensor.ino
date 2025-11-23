#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>

// ---------- WIFI ----------
#define WIFI_SSID     "Kosala's Galaxy A21S"
#define WIFI_PASSWORD "11111111"

// ---------- FIREBASE ----------
#define API_KEY      "AIzaSyCSEWW0wawOzJgvxyOmDqXMqE8nihprCd0"
#define DATABASE_URL "https://iot-project-3c897-default-rtdb.asia-southeast1.firebasedatabase.app"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// For parsing serial lines
String inputLine = "";

// Forward declaration
void processLine(String line);

void setup() {
  Serial.begin(9600);       // 🔁 Connected to Arduino TX
  delay(500);

  // ---- WiFi ----
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP: ");
  Serial.println(WiFi.localIP());

  // ---- Firebase ----
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase signUp OK ✅");
  } else {
    Serial.printf("signUp error: %s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("ESP8266 ready. Waiting for sensor data from Arduino...");
}

void loop() {
  if (!Firebase.ready()) {
    return;
  }

  // ---- Read line from Arduino ----
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      // Full line received
      processLine(inputLine);
      inputLine = "";
    } else if (c != '\r') {
      inputLine += c;
    }
  }
}

void processLine(String line) {
  line.trim();
  if (line.length() == 0) return;

  Serial.print("Got line: ");
  Serial.println(line);

  // ------------------------------------------------
  // 1) SOIL SENSOR LINE
  //     Format: SOIL,moisture=45,level=LOW
  // ------------------------------------------------
  if (line.startsWith("SOIL")) {
    float moistureValue = 0;
    String levelValue = "";

    int mIndex = line.indexOf("moisture=");
    if (mIndex != -1) {
      int commaIndex = line.indexOf(",", mIndex);
      String mStr;
      if (commaIndex == -1) {
        mStr = line.substring(mIndex + 9);  // after "moisture="
      } else {
        mStr = line.substring(mIndex + 9, commaIndex);
      }
      moistureValue = mStr.toFloat();
    }

    int levelIndex = line.indexOf("level=");
    if (levelIndex != -1) {
      levelValue = line.substring(levelIndex + 6); // after "level="
    }

    Serial.print("SOIL Parsed → Moisture: ");
    Serial.print(moistureValue);
    Serial.print(" | Level: ");
    Serial.println(levelValue);

    // Upload to Firebase
    if (!Firebase.RTDB.setFloat(&fbdo, "device1/soil/moisture", moistureValue)) {
      Serial.print("Firebase error (soil/moisture): ");
      Serial.println(fbdo.errorReason());
    }
    if (!Firebase.RTDB.setString(&fbdo, "device1/soil/level", levelValue)) {
      Serial.print("Firebase error (soil/level): ");
      Serial.println(fbdo.errorReason());
    }

    return;   // done with SOIL line
  }

  // ------------------------------------------------
  // 2) ACCELEROMETER LINES
  //     Format: ACCEL1,tilt=23.45,level=LOW
  //             ACCEL2,tilt=...,level=...
  //             ACCEL3,tilt=...,level=...
  // ------------------------------------------------
  if (!line.startsWith("ACCEL")) {
    // not soil, not accel → ignore
    Serial.println("Unknown line type, ignored.");
    return;
  }

  // Detect which accelerometer
  int accelId = 0;   // 0 = invalid
  if (line.startsWith("ACCEL1")) {
    accelId = 1;
  } else if (line.startsWith("ACCEL2")) {
    accelId = 2;
  } else if (line.startsWith("ACCEL3")) {
    accelId = 3;
  } else {
    Serial.println("⚠ Unknown ACCEL ID");
    return;
  }

  // Parse tilt and level
  float tiltValue = 0;
  String levelValue = "";

  int tiltIndex = line.indexOf("tilt=");
  if (tiltIndex != -1) {
    int commaIndex = line.indexOf(",", tiltIndex);
    String tiltStr;
    if (commaIndex == -1) {
      tiltStr = line.substring(tiltIndex + 5);  // after "tilt="
    } else {
      tiltStr = line.substring(tiltIndex + 5, commaIndex);
    }
    tiltValue = tiltStr.toFloat();
  }

  int levelIndex = line.indexOf("level=");
  if (levelIndex != -1) {
    levelValue = line.substring(levelIndex + 6); // after "level="
  }

  Serial.print("ACCEL");
  Serial.print(accelId);
  Serial.print(" Parsed → Tilt: ");
  Serial.print(tiltValue);
  Serial.print(" | Level: ");
  Serial.println(levelValue);

  // Build Firebase base path:
  //  device1/accelerometer1
  //  device1/accelerometer2
  //  device1/accelerometer3
  String basePath = "device1/accelerometer";
  basePath += String(accelId);

  // Upload to Firebase
  if (!Firebase.RTDB.setFloat(&fbdo, basePath + "/tilt", tiltValue)) {
    Serial.print("Firebase error (tilt): ");
    Serial.println(fbdo.errorReason());
  }
  if (!Firebase.RTDB.setString(&fbdo, basePath + "/level", levelValue)) {
    Serial.print("Firebase error (level): ");
    Serial.println(fbdo.errorReason());
  }
}
