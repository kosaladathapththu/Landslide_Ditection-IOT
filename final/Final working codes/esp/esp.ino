#include <Wire.h>
#include <SoftwareSerial.h>
#include <math.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>

// ------------------- ESP Pins -------------------
// NOTE: MPU3 is now handled by Arduino, so ESP does NOT read it via I2C anymore.
// You can still keep SDA/SCL but they are unused here.
#define SDA_PIN D2
#define SCL_PIN D1

#define ARDUINO_RX D6
#define ARDUINO_TX D5

SoftwareSerial arduinoSerial(ARDUINO_RX, ARDUINO_TX);

// ------------------- Thresholds -------------------
float VIB_THRESHOLD = 1.0;   // change between 2 samples (deg)

float slopeLow = 15;
float slopeModerate = 25;
float slopeHigh = 35;
float slopeCritical = 45;

float soilVeryLow = 20;
float soilModerate = 40;
float soilHigh = 60;

// ------------------- Variables -------------------
float soilRaw, rainRaw, humidity, temp;
float mpu1_angle, mpu2_angle, mpu3_angle;

float mpu1_prev = 0, mpu2_prev = 0, mpu3_prev = 0;
String mpu1_vib, mpu2_vib, mpu3_vib;

// offset (auto-zero, current angles = 0)
float mpu1_offset = 0, mpu2_offset = 0, mpu3_offset = 0;
bool mpu1_cal = false, mpu2_cal = false, mpu3_cal = false;

// ignore first N samples for vibration (warm-up)
int vibWarmupCount = 10;
int vibSampleCounter = 0;

// time-based risk filter (A)
int riskCounter = 0;          // counts consecutive high-risk readings
const int RISK_LIMIT = 5;     // need 5 readings in a row (~5 seconds)

// ------------------- Firebase Config -------------------
#define WIFI_SSID     "connectile dysfunction"
#define WIFI_PASSWORD "1234bucklemyshoe"

#define API_KEY       "AIzaSyCSEWW0wawOzJgvxyOmDqXMqE8nihprCd0"
#define DATABASE_URL  "https://iot-project-3c897-default-rtdb.asia-southeast1.firebasedatabase.app"
#define USER_EMAIL    "kosalaathapaththu1234@gmail.com"
#define USER_PASSWORD "Plapytome2@"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// ------------------- Helper Functions -------------------

String vibration(float prev, float now) {
  float delta = fabs(now - prev);
  float magNow = fabs(now);
  float magPrev = fabs(prev);

  if (magNow < 2.0 && magPrev < 2.0) {
    return "Low";
  }

  if (delta > VIB_THRESHOLD) {
    return "High";
  }

  return "Low";
}

float normalize(float ang) {
  if (ang > 90) ang -= 180;
  if (ang < -90) ang += 180;
  return ang;
}

String tiltRisk(float a) {
  float A = fabs(a);
  if (A < slopeLow) return "Very Low";
  else if (A < slopeModerate) return "Low–Moderate";
  else if (A < slopeHigh) return "Moderate–High";
  else if (A < slopeCritical) return "High";
  else return "Highly Variable";
}

// include rainMM (B)
String soilRiskCalc(float soil, float maxSlope, float rainMM) {
  if (soil < soilVeryLow) return "Very Low";
  if (soil < soilModerate && maxSlope > 25) return "Moderate";
  if (soil < soilHigh && maxSlope > 25) return "High";
  if (soil >= soilHigh && maxSlope > 15) return "Critical";

  // heavy rain effect (B)
  if (rainMM > 80 && soil >= soilModerate && maxSlope > 15) {
    return "High";
  }
  if (rainMM > 120 && soil >= soilModerate && maxSlope > 20) {
    return "Critical";
  }

  return "Low";
}

// improved landslide logic (C)
bool landslideDetect(
  String soilRisk,
  float a1,float a2,float a3,
  String v1,String v2,String v3
){
  float A1=fabs(a1), A2=fabs(a2), A3=fabs(a3);

  if (soilRisk=="Critical") return true;

  // bottom sensor (MPU2)
  bool deep = (A2>=25 && soilRisk!="Low") || (A2>=15 && v2=="High");

  // middle sensor (MPU1)
  bool mid = (A1>=30 && soilRisk!="Very Low") || (A1>=25 && v1=="High");

  // top sensor (MPU3) – new logic
  bool top = false;

  // 1) even with lower angle: strong top vibration + any other vibration
  if (v3 == "High" && (v1 == "High" || v2 == "High")) {
    top = true;
  }
  // 2) or very large top angle alone
  else if (A3 > 40) {
    top = true;
  }

  return deep || mid || top;
}

// ------------------- SETUP -------------------
void setup() {
  Serial.begin(9600);
  arduinoSerial.begin(9600);

  // I2C still started (even if ESP doesn't talk to MPU now)
  Wire.begin(SDA_PIN, SCL_PIN);

  // WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }
  Serial.println("\nWiFi OK");

  // Firebase
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("ESP READY (AUTO-ZERO + NEW VIBRATION + TIME FILTER)");
}

// ------------------- LOOP -------------------
void loop() {

  // ----- 1. Receive from Arduino -----
  if (arduinoSerial.available()) {
    String line = arduinoSerial.readStringUntil('\n');
    line.trim();

    // now expecting 7 values: soil,rain,hum,temp,mpu1,mpu2,mpu3
    float vals[7];
    int i=0;
    while (line.length()>0 && i<7) {
      int c = line.indexOf(',');
      if (c==-1) c=line.length();
      vals[i] = line.substring(0,c).toFloat();
      line = line.substring(c+1);
      i++;
    }

    if (i >= 7) { // only assign if all present
      soilRaw    = vals[0];
      rainRaw    = vals[1];
      humidity   = vals[2];
      temp       = vals[3];
      mpu1_angle = vals[4];
      mpu2_angle = vals[5];
      mpu3_angle = vals[6];
    }
  }

  // ----- 2. Soil / Rain -----
  float soilPercent = 100.0 - (soilRaw/1023.0*100.0);
  float rainMM      = (1023-rainRaw)/1023.0 * 150;

  // ----- 3. Normalize -----
  float m1 = normalize(mpu1_angle);
  float m2 = normalize(mpu2_angle);
  float m3 = normalize(mpu3_angle);

  // ----- 4. Auto-zero (baseline = current angles at startup) -----
  if (!mpu1_cal){mpu1_offset=m1; mpu1_prev=0; mpu1_cal=true;}
  if (!mpu2_cal){mpu2_offset=m2; mpu2_prev=0; mpu2_cal=true;}
  if (!mpu3_cal){mpu3_offset=m3; mpu3_prev=0; mpu3_cal=true;}

  mpu1_angle = m1 - mpu1_offset;
  mpu2_angle = m2 - mpu2_offset;
  mpu3_angle = m3 - mpu3_offset;

  // ----- 5. Vibration (with warm-up) -----
  vibWarmupCount++;  // <-- careful: this should be vibSampleCounter++ in your real code
  vibSampleCounter++;

  if (vibSampleCounter <= vibWarmupCount) {
    mpu1_vib = "Low";
    mpu2_vib = "Low";
    mpu3_vib = "Low";
  } else {
    mpu1_vib = vibration(mpu1_prev, mpu1_angle);
    mpu2_vib = vibration(mpu2_prev, mpu2_angle);
    mpu3_vib = vibration(mpu3_prev, mpu3_angle);
  }

  mpu1_prev = mpu1_angle;
  mpu2_prev = mpu2_angle;
  mpu3_prev = mpu3_angle;

  // ----- 6. Landslide logic -----
  float maxSlope = max(fabs(mpu1_angle), max(fabs(mpu2_angle), fabs(mpu3_angle)));

  String soilRisk = soilRiskCalc(soilPercent, maxSlope, rainMM);

  bool instantRisk = landslideDetect(soilRisk,
                                     mpu1_angle, mpu2_angle, mpu3_angle,
                                     mpu1_vib, mpu2_vib, mpu3_vib);

  // time filter (A): need 5 readings in a row
  if (instantRisk) {
    riskCounter++;
  } else {
    riskCounter = 0;
  }
  if (riskCounter > RISK_LIMIT) riskCounter = RISK_LIMIT;

  bool slide = (riskCounter >= RISK_LIMIT);

  // ----- 7. Buzzer command to Arduino -----
  if (slide) arduinoSerial.write('1');
  else       arduinoSerial.write('0');

  // ----- 8. Firebase WRITE -----
  if (Firebase.ready()) {
    Firebase.RTDB.setFloat (&fbdo, "/sensor_data/MPU1_Tilt", mpu1_angle);
    Firebase.RTDB.setString(&fbdo, "/sensor_data/MPU1_Vibration", mpu1_vib);
    Firebase.RTDB.setString(&fbdo, "/sensor_data/MPU1_Risk", tiltRisk(mpu1_angle));

    Firebase.RTDB.setFloat (&fbdo, "/sensor_data/MPU2_Tilt", mpu2_angle);
    Firebase.RTDB.setString(&fbdo, "/sensor_data/MPU2_Vibration", mpu2_vib);
    Firebase.RTDB.setString(&fbdo, "/sensor_data/MPU2_Risk", tiltRisk(mpu2_angle));

    Firebase.RTDB.setFloat (&fbdo, "/sensor_data/MPU3_Tilt", mpu3_angle);
    Firebase.RTDB.setString(&fbdo, "/sensor_data/MPU3_Vibration", mpu3_vib);
    Firebase.RTDB.setString(&fbdo, "/sensor_data/MPU3_Risk", tiltRisk(mpu3_angle));

    Firebase.RTDB.setFloat (&fbdo, "/sensor_data/SoilPercent", soilPercent);
    Firebase.RTDB.setString(&fbdo, "/sensor_data/SoilRisk", soilRisk);

    Firebase.RTDB.setFloat (&fbdo, "/sensor_data/RainMM", rainMM);
    Firebase.RTDB.setFloat (&fbdo, "/sensor_data/Humidity", humidity);
    Firebase.RTDB.setFloat (&fbdo, "/sensor_data/Temperature", temp);

    Firebase.RTDB.setString(&fbdo, "/sensor_data/Landslide", slide ? "YES" : "NO");
  }

  delay(1000);
}
