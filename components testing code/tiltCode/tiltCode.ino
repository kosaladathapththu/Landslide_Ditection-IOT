#include <Wire.h>
#include <MPU6050.h>
#include <math.h>

// ✅ Use explicit I2C address that scanner showed (0x68)
MPU6050 accelerometer(0x68);

// Tilt levels (in degrees)
const float lowTiltAngle  = 10.0;
const float midTiltAngle  = 20.0;
const float highTiltAngle = 180.0;

  // For NodeMCU: SDA = D2, SCL = D1
  Wire.begin(D2, D1);
  // (Optional) faster I2C:
  // Wire.setClock(400000);

  Serial.println("Initializing Accelerometer...");
  accelerometer.initialize();

  // ✅ Do NOT loop forever. Just check once and warn.
  if (!accelerometer.testConnection()) {
    Serial.println("⚠ Warning: testConnection failed, but I2C device found at 0x68. Continuing anyway...");
  } else {
    Serial.println("Accelerometer Connected :)");
  }
}

void loop() {
  // Read raw acceleration
  accelerometer.getAcceleration(&axisX, &axisY, &axisZ);

  // Convert to g-force
  xAxisGforce = axisX / gForce;
  yAxisGforce = axisY / gForce;
  zAxisGforce = axisZ / gForce;

  // Calculate angles (in degrees)
  roll = atan2(yAxisGforce, zAxisGforce) * 180.0 / PI;
  pitch = atan2(-xAxisGforce, sqrt(yAxisGforce * yAxisGforce + zAxisGforce * zAxisGforce)) * 180.0 / PI;

  // Use the maximum tilt from roll or pitch
  float tilt = max(abs(roll), abs(pitch));

  // Debug
  Serial.print("Roll: ");
  Serial.print(roll);
  Serial.print(" | Pitch: ");
  Serial.print(pitch);
  Serial.print(" | Max Tilt: ");
  Serial.println(tilt);

  // Decide tilt level
  if (tilt >= highTiltAngle) {
    Serial.println("High Tilting Alert");
  } else if (tilt >= midTiltAngle) {
    Serial.println("Mid Tilting Alert");
  } else if (tilt >= lowTiltAngle) {
    Serial.println("Low Tilting Alert");
  } else {
    Serial.println("No tilting");
  }

  delay(1000);  // small delay for stability
}
