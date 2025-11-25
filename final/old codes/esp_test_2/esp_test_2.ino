#include <Wire.h>
#include <SoftwareSerial.h>
#include <math.h>

// ------------------- Pins -------------------
#define MPU3_ADDR 0x68
#define SDA_PIN D2
#define SCL_PIN D1

#define ARDUINO_RX D6  // ESP receives from Arduino TX
#define ARDUINO_TX D5  // ESP TX to Arduino RX (not used now)

SoftwareSerial arduinoSerial(ARDUINO_RX, ARDUINO_TX); // RX, TX

// ------------------- Thresholds -------------------
float slopeLow = 15;
float slopeModerate = 25;
float slopeHigh = 35;
float slopeCritical = 45;

float soilVeryLow = 20;
float soilModerate = 40;
float soilHigh = 60;

float rainWatch = 75;
float rainAlert = 100;
float rainEvacuation = 150;

// ------------------- Variables -------------------
float soilRaw, rainRaw, humidity, temp;
float mpu1_angle, mpu2_angle, mpu3_angle;

// ------------------- Setup -------------------
void setup() {
  Serial.begin(9600);             
  arduinoSerial.begin(9600);      
  Wire.begin(SDA_PIN, SCL_PIN);   

  // Initialize MPU3
  Wire.beginTransmission(MPU3_ADDR);
  Wire.write(0x6B); 
  Wire.write(0);
  Wire.endTransmission(true);

  Serial.println("ESP Ready!");
}

// ------------------- Loop -------------------
void loop() {
  // 1. Read data from Arduino
  if (arduinoSerial.available()) {
    String data = arduinoSerial.readStringUntil('\n');
    parseArduinoData(data);
  }

  // 2. Read MPU3 tilt
  mpu3_angle = readMPU(MPU3_ADDR);

  // 3. Convert raw values to real units (inverted for correct behavior)
  float soilPercent = 100.0 - (soilRaw / 1023.0 * 100.0);  // Wet soil = higher %
  float rainMM = (1023 - rainRaw) / 1023.0 * 150.0;        // More rain = higher mm

  // 4. Normalize MPU angles
  mpu1_angle = normalizeAngle(mpu1_angle);
  mpu2_angle = normalizeAngle(mpu2_angle);
  mpu3_angle = normalizeAngle(mpu3_angle);

  // 5. Print risks
  printMPURisk("MPU1", mpu1_angle);
  printMPURisk("MPU2", mpu2_angle);
  printMPURisk("MPU3", mpu3_angle);

  printSoilRisk(soilPercent, mpu1_angle);  // Use MPU1 tilt for soil logic
  printRainRisk(rainMM);
  printHumidity();

  delay(1000);
}

// ------------------- Functions -------------------

// Parse CSV from Arduino
void parseArduinoData(String data) {
  int index = 0;
  float values[6];
  while (data.length() > 0 && index < 6) {
    int comma = data.indexOf(',');
    if (comma == -1) comma = data.length();
    values[index] = data.substring(0, comma).toFloat();
    data = data.substring(comma + 1);
    index++;
  }
  soilRaw = values[0];
  rainRaw = values[1];
  humidity = values[2];
  temp = values[3];
  mpu1_angle = values[4];
  mpu2_angle = values[5];
}

// Read MPU tilt (X-axis)
float readMPU(int addr) {
  Wire.beginTransmission(addr);
  Wire.write(0x3B); 
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)addr, (uint8_t)6, (uint8_t)true);

  int16_t ax = Wire.read() << 8 | Wire.read();
  int16_t ay = Wire.read() << 8 | Wire.read();
  int16_t az = Wire.read() << 8 | Wire.read();

  return atan2(ay, az) * 57.3;
}

// Normalize angle to -90 to +90 for risk calculation
float normalizeAngle(float angle) {
  if (angle > 90) angle -= 180;
  if (angle < -90) angle += 180;
  return angle;
}

// Print MPU tilt risk
void printMPURisk(String name, float angle) {
  String risk = "";
  if (angle < slopeLow) risk = "Very Low";
  else if (angle < slopeModerate) risk = "Low-Moderate";
  else if (angle < slopeHigh) risk = "Moderate-High";
  else if (angle < slopeCritical) risk = "High";
  else risk = "Highly Variable";

  Serial.println(name + " Tilt: " + String(angle,1) + "°, Risk: " + risk);
}

// Print soil risk
void printSoilRisk(float soil, float tilt) {
  String risk = "Very Low";
  if (soil < soilVeryLow) risk = "Very Low";
  else if (soil < soilModerate && tilt > 25) risk = "Moderate";
  else if (soil < soilHigh && tilt > 25) risk = "High";
  else if (soil >= soilHigh && tilt > 15) risk = "Critical";

  Serial.println("Soil Moisture: " + String(soil,1) + "%, Risk: " + risk);
}

// Print rain risk
void printRainRisk(float rain) {
  String risk = "Low";
  if (rain >= rainEvacuation) risk = "High/Evacuation";
  else if (rain >= rainAlert) risk = "Moderate/Alert";
  else if (rain >= rainWatch) risk = "Low/Watch";

  Serial.println("Rain: " + String(rain,1) + "mm/day, Risk: " + risk);
}

// Print humidity
void printHumidity() {
  Serial.println("Humidity: " + String(humidity,1) + "%");
}
