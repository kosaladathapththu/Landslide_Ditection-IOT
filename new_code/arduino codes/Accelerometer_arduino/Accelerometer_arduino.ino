#include <Wire.h>
#include <MPU6050.h>
#include <math.h>

MPU6050 accel(0x68);   // address 0x68

const float gForce = 16384.0;  // 1g = 16384 (±2g)

// Tilt thresholds (degrees)
const float lowTiltAngle   = 10.0;
const float midTiltAngle   = 20.0;
const float highTiltAngle  = 45.0;

// Vibration thresholds (g-change per sample)
const float lowVibThresh   = 0.05;  // small vibration
const float midVibThresh   = 0.12;
const float highVibThresh  = 0.25;

// Previous values for vibration calculation
float prevMag = 1.0;  // gravity ~1g
bool  firstSample = true;

unsigned long lastSampleMs = 0;

void setup() {
  Serial.begin(9600);  // to ESP8266
  Wire.begin();

  Serial.println("Initializing MPU6050...");
  accel.initialize();
  if (!accel.testConnection()) {
    Serial.println("⚠ MPU6050 connection FAILED");
  } else {
    Serial.println("✅ MPU6050 OK");
  }
}

void loop() {
  unsigned long now = millis();

  // Sample every 100 ms (10 Hz)
  if (now - lastSampleMs < 100) {
    return;
  }
  lastSampleMs = now;

  // ---- 1) Read raw acceleration ----
  int16_t ax, ay, az;
  accel.getAcceleration(&ax, &ay, &az);

  // Convert to g-force
  float gx = ax / gForce;
  float gy = ay / gForce;
  float gz = az / gForce;

  // ---- 2) Tilt calculation ----
  float roll  = atan2(gy, gz) * 180.0 / PI;
  float pitch = atan2(-gx, sqrt(gy * gy + gz * gz)) * 180.0 / PI;
  float tilt  = max(fabs(roll), fabs(pitch));  // max tilt

  const char* tiltLevel = "NONE";
  if (tilt >= highTiltAngle) {
    tiltLevel = "HIGH";
  } else if (tilt >= midTiltAngle) {
    tiltLevel = "MID";
  } else if (tilt >= lowTiltAngle) {
    tiltLevel = "LOW";
  } else {
    tiltLevel = "NONE";
  }

  // ---- 3) Vibration (earthquake) calculation ----
  // Use change in acceleration magnitude as “vibration strength”
  float mag = sqrt(gx * gx + gy * gy + gz * gz);  // total g
  float vib = 0.0;

  if (!firstSample) {
    vib = fabs(mag - prevMag);  // change since last sample
  } else {
    firstSample = false;
  }
  prevMag = mag;

  const char* vibLevel = "NONE";
  if (vib >= highVibThresh) {
    vibLevel = "HIGH";
  } else if (vib >= midVibThresh) {
    vibLevel = "MID";
  } else if (vib >= lowVibThresh) {
    vibLevel = "LOW";
  } else {
    vibLevel = "NONE";
  }

  // ---- 4) Debug prints (optional) ----
  Serial.print("Tilt=");
  Serial.print(tilt);
  Serial.print(" (");
  Serial.print(tiltLevel);
  Serial.print(") | Vib=");
  Serial.print(vib, 3);
  Serial.print(" (");
  Serial.print(vibLevel);
  Serial.println(")");

  // ---- 5) SEND CLEAN LINE TO ESP8266 ----
  // Format:
  // ACCEL1,tilt=23.45,tiltLevel=MID,vib=0.12,vibLevel=LOW
  Serial.print("ACCEL1,tilt=");
  Serial.print(tilt, 2);
  Serial.print(",tiltLevel=");
  Serial.print(tiltLevel);
  Serial.print(",vib=");
  Serial.print(vib, 3);
  Serial.print(",vibLevel=");
  Serial.println(vibLevel);
}
