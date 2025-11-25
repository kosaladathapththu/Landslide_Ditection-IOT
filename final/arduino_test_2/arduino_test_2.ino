#include <Wire.h>
#include <DHT.h>
#include <SoftwareSerial.h>  // For serial communication on pins 2 and 3

#define DHTPIN 7
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Soil & Rain
#define SOIL_PIN A0
#define RAIN_PIN A1

// MPU6050 addresses
#define MPU1_ADDR 0x68
#define MPU2_ADDR 0x69

// SoftwareSerial for ESP communication
#define RX_PIN 3  // Arduino receives (not used here)
#define TX_PIN 2  // Arduino transmits
SoftwareSerial espSerial(RX_PIN, TX_PIN);

void setup() {
  Serial.begin(9600);       // For monitoring via USB
  espSerial.begin(9600);    // For sending data to ESP
  Wire.begin();
  dht.begin();

  // Initialize MPU1
  Wire.beginTransmission(MPU1_ADDR);
  Wire.write(0x6B); // power management register
  Wire.write(0);
  Wire.endTransmission(true);

  // Initialize MPU2
  Wire.beginTransmission(MPU2_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
}

void loop() {
  // Read soil and rain sensors
  int soil = analogRead(SOIL_PIN);
  int rain = analogRead(RAIN_PIN);

  // Read DHT22
  float hum = dht.readHumidity();
  float temp = dht.readTemperature();

  // Read MPU1 and MPU2 tilt (X-axis angle)
  float mpu1_angle = readMPU(MPU1_ADDR);
  float mpu2_angle = readMPU(MPU2_ADDR);

  // Send all values via SoftwareSerial as CSV to ESP
  // Format: soil,rain,humidity,temp,mpu1_angle,mpu2_angle
  espSerial.print(soil); espSerial.print(",");
  espSerial.print(rain); espSerial.print(",");
  espSerial.print(hum); espSerial.print(",");
  espSerial.print(temp); espSerial.print(",");
  espSerial.print(mpu1_angle); espSerial.print(",");
  espSerial.println(mpu2_angle);

  // Also print to USB Serial for monitoring
  Serial.print("Sent to ESP: ");
  Serial.print(soil); Serial.print(",");
  Serial.print(rain); Serial.print(",");
  Serial.print(hum); Serial.print(",");
  Serial.print(temp); Serial.print(",");
  Serial.print(mpu1_angle); Serial.print(",");
  Serial.println(mpu2_angle);

  delay(1000); // 1 second
}

// Simple function to read X-axis tilt of MPU
float readMPU(int addr) {
  Wire.beginTransmission(addr);
  Wire.write(0x3B); // starting register for accel data
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)addr, (uint8_t)6, (uint8_t)true);

  int16_t ax = Wire.read() << 8 | Wire.read();
  int16_t ay = Wire.read() << 8 | Wire.read();
  int16_t az = Wire.read() << 8 | Wire.read();

  // Simple tilt angle in degrees
  float angle = atan2(ay, az) * 57.3;
  return angle;
}
