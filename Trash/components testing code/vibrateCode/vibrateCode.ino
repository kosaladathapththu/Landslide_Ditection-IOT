#include <Wire.h>
#include <MPU6050.h>
#include <math.h>

MPU6050 accelerometer(0x68);

// Vibration Thresholds
const float lowVibrateFrequency = 0.10;
const float midVibrateFrequency = 0.30;
const float highVibrateFrequency = 0.60;

// Raw accelerometer values
short int axisX, axisY, axisZ;

// Previous values
float axisXPrevious = 0;
float axisYPrevious = 0;
float axisZPrevious = 0;

// G-force values
float xAxisGforce, yAxisGforce, zAxisGforce;
const float gForce = 16384.0;

// Delta vibration
float delta;

// Detection flags
bool lowVibrateDetection;
bool midVibrateDetection;
bool highVibrateDetection;

void setup() {
  Serial.begin(9600);

  // For NodeMCU: SDA = D2, SCL = D1
  Wire.begin(D2, D1);

  Serial.println("Initializing Accelerometer...");
  accelerometer.initialize();

  if (!accelerometer.testConnection()) {
    Serial.println("⚠ Warning: testConnection failed, but device detected on I2C 0x68");
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

  // Calculate vibration
  delta = sqrt(
            pow(xAxisGforce - axisXPrevious, 2) +
            pow(yAxisGforce - axisYPrevious, 2) +
            pow(zAxisGforce - axisZPrevious, 2)
          );

  // Update previous values
  axisXPrevious = xAxisGforce;
  axisYPrevious = yAxisGforce;
  axisZPrevious = zAxisGforce;

  // Detection levels
  highVibrateDetection = (delta > highVibrateFrequency);
  midVibrateDetection  = (delta > midVibrateFrequency);
  lowVibrateDetection  = (delta > lowVibrateFrequency);

  if (highVibrateDetection) {
    Serial.println("High Vibration Alert!");
  } 
  else if (midVibrateDetection) {
    Serial.println("Mid Vibration Alert!");
  } 
  else if (lowVibrateDetection) {
    Serial.println("Low Vibration Alert!");
  } 
  else {
    Serial.println("No vibration");
  }

  delay(100);
}
