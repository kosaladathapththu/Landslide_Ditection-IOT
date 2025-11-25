#include <Wire.h>
#include <SoftwareSerial.h>
#include <math.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>

#define MPU3_ADDR 0x68
#define SDA_PIN D2
#define SCL_PIN D1

#define ARDUINO_RX D6
#define ARDUINO_TX D5

SoftwareSerial arduinoSerial(ARDUINO_RX, ARDUINO_TX);

float VIB_THRESHOLD = 1.0;   

float slopeLow = 15;
float slopeModerate = 25;
float slopeHigh = 35;
float slopeCritical = 45;
float soilVeryLow = 20;
float soilModerate = 40;
float soilHigh = 60;
float soilRaw, rainRaw, humidity, temp;
float mpu1_angle, mpu2_angle, mpu3_angle;
float mpu1_prev = 0, mpu2_prev = 0, mpu3_prev = 0;
String mpu1_vib, mpu2_vib, mpu3_vib;
float mpu1_offset = 0, mpu2_offset = 0, mpu3_offset = 0;
bool mpu1_cal = false, mpu2_cal = false, mpu3_cal = false;
int vibWarmupCount = 10;
int vibSampleCounter = 0;

#define WIFI_SSID     "connectile dysfunction"
#define WIFI_PASSWORD "1234bucklemyshoe"
#define API_KEY       "AIzaSyCSEWW0wawOzJgvxyOmDqXMqE8nihprCd0"
#define DATABASE_URL  "https://iot-project-3c897-default-rtdb.asia-southeast1.firebasedatabase.app"
#define USER_EMAIL    "kosalaathapaththu1234@gmail.com"
#define USER_PASSWORD "Plapytome2@"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;


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

String soilRiskCalc(float soil, float maxSlope) {
  if (soil < soilVeryLow) return "Very Low";
  if (soil < soilModerate && maxSlope > 25) return "Moderate";
  if (soil < soilHigh && maxSlope > 25) return "High";
  if (soil >= soilHigh && maxSlope > 15) return "Critical";
  return "Low";
}

bool landslideDetect(
  String soilRisk,
  float a1,float a2,float a3,
  String v1,String v2,String v3
){
  float A1=fabs(a1), A2=fabs(a2), A3=fabs(a3);

  if (soilRisk=="Critical") return true;

  bool deep = (A2>=25 && soilRisk!="Low") || (A2>=15 && v2=="High");

  bool mid = (A1>=30 && soilRisk!="Very Low") || (A1>=25 && v1=="High");

  bool top = (A3>=35 && v3=="High") || (A3>45);

  return deep || mid || top;
}

void setup() {
  Serial.begin(9600);
  arduinoSerial.begin(9600);
  Wire.begin(SDA_PIN, SCL_PIN);

  Wire.beginTransmission(MPU3_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }
  Serial.println("\nWiFi OK");

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("ESP READY (AUTO-ZERO + NEW VIBRATION)");
}

void loop() {

  if (arduinoSerial.available()) {
    String line = arduinoSerial.readStringUntil('\n');
    line.trim();

    float vals[6];
    int i=0;
    while (line.length()>0 && i<6) {
      int c = line.indexOf(',');
      if (c==-1) c=line.length();
      vals[i] = line.substring(0,c).toFloat();
      line = line.substring(c+1);
      i++;
    }

    soilRaw   = vals[0];
    rainRaw   = vals[1];
    humidity  = vals[2];
    temp      = vals[3];
    mpu1_angle= vals[4];
    mpu2_angle= vals[5];
  }

  mpu3_angle = readMPU(MPU3_ADDR);

  float soilPercent = 100.0 - (soilRaw/1023.0*100.0);
  float rainMM      = (1023-rainRaw)/1023.0 * 150;

  float m1 = normalize(mpu1_angle);
  float m2 = normalize(mpu2_angle);
  float m3 = normalize(mpu3_angle);

  if (!mpu1_cal){mpu1_offset=m1; mpu1_prev=0; mpu1_cal=true;}
  if (!mpu2_cal){mpu2_offset=m2; mpu2_prev=0; mpu2_cal=true;}
  if (!mpu3_cal){mpu3_offset=m3; mpu3_prev=0; mpu3_cal=true;}

  mpu1_angle = m1 - mpu1_offset;
  mpu2_angle = m2 - mpu2_offset;
  mpu3_angle = m3 - mpu3_offset;

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

  float maxSlope = max(fabs(mpu1_angle), max(fabs(mpu2_angle), fabs(mpu3_angle)));
  String soilRisk = soilRiskCalc(soilPercent, maxSlope);
  bool slide = landslideDetect(soilRisk,
                               mpu1_angle, mpu2_angle, mpu3_angle,
                               mpu1_vib, mpu2_vib, mpu3_vib);

  if (slide) arduinoSerial.write('1');
  else       arduinoSerial.write('0');

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

float readMPU(int addr){
  Wire.beginTransmission(addr);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(addr, 6, true);

  int16_t ax = Wire.read()<<8 | Wire.read();
  int16_t ay = Wire.read()<<8 | Wire.read();
  int16_t az = Wire.read()<<8 | Wire.read();

  return atan2(ay, az)*57.3;
}