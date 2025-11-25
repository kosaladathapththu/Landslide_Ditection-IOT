#include <Wire.h>
#include <DHT.h>
#include <SoftwareSerial.h>

#define DHTPIN 7
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Soil & Rain pins
#define SOIL_PIN A2
#define RAIN_PIN A3

// MPU addresses
#define MPU1_ADDR 0x68
#define MPU2_ADDR 0x69

// Serial to ESP8266
#define RX_PIN 3
#define TX_PIN 2
SoftwareSerial espSerial(RX_PIN, TX_PIN);

void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);
  Wire.begin();
  dht.begin();

  // Init MPU1
  Wire.beginTransmission(MPU1_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  // Init MPU2
  Wire.beginTransmission(MPU2_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  Serial.println("Arduino ready.");
}

void loop() {
  int soil = analogRead(SOIL_PIN);
  int rain = analogRead(RAIN_PIN);

  float hum  = dht.readHumidity();
  float temp = dht.readTemperature();

  float mpu1_angle = readMPU(MPU1_ADDR);
  float mpu2_angle = readMPU(MPU2_ADDR);

  // Create CSV
  String csv = String(soil) + "," + String(rain) + "," +
               String(hum) + "," + String(temp) + "," +
               String(mpu1_angle) + "," + String(mpu2_angle);

  espSerial.println(csv);
  Serial.println("Sent to ESP: " + csv);

  delay(1000);
}

// Read MPU angle
float readMPU(int addr) {
  Wire.beginTransmission(addr);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(addr, 6, true);

  int16_t ax = Wire.read()<<8 | Wire.read();
  int16_t ay = Wire.read()<<8 | Wire.read();
  int16_t az = Wire.read()<<8 | Wire.read();

  return atan2(ay, az) * 57.3;
}
