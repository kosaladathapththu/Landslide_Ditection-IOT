#include <Wire.h>
#include <MPU6050.h>
#include <math.h>

#define tcaAdress 0x70

MPU6050 acelerometer(0x68);

const float lowTitleAngle = 10.0;
const float midTitleAngle = 10.0;
const float highTitleAngle = 10.0;
const float lowVibrateFrequency = 0.10;
const float midVibrateFrequency = 0.30;
const float highVibrateFrequency = 0.60;
const float gForce = 16384.0; 

short int axisX, axisY, axisZ;

float axisXPrevious[3] = {0, 0, 0};
float axisYPrevious[3] = {0, 0, 0};
float axisZPrevious[3] = {0, 0, 0};

float xAxisGforce, yAxisGforce, zAxisGforce;

float roll, pitch, tilt, delta;

String groundAcelerometer, midAcelerometer, lowAcelerometer;

void setup() {
  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:

}

void selectAcelerometerChannel(unit8_8 channel) {
  Wire.beginTransmission(tcaAdress);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

String readTilt(int sensorNumber) {
  selectAcelerometerChannel(sensorNumber);
  acelerometer.getAcceleration(&axisX, &axisY, &axisZ);
  xAxisGforce = axisX / gForce;
  yAxisGforce = axisY / gForce;
  zAxisGforce = axisZ / gForce;
  roll = atan2(yAxisGforce, zAxisGforce) * 180.0 / PI;
  pitch = atan2(-xAxisGforce, sqrt(yAxisGforce * yAxisGforce + zAxisGforce * zAxisGforce)) * 180.0 / PI;
  tilt = max(abs(roll), abs(pitch));
  if (tilt >= highTiltAngle) {
    Serial.println("High Tilting Alert");
    return "HIGH";
  } else if (tilt >= midTiltAngle) {
    Serial.println("Mid Tilting Alert");
    return "MID"
  } else if (tilt >= lowTiltAngle) {
    Serial.println("Low Tilting Alert");
    return "LOW"
  } else {
    Serial.println("No tilting");
    return "NO"
  }
}

String readVibration(int sensorNumber) {
  selectAcelerometerChannel(sensorNumber);
  acelerometer.getAcceleration(&axisX, &axisY, &axisZ);
  xAxisGforce = axisX / gForce;
  yAxisGforce = axisY / gForce;
  zAxisGforce = axisZ / gForce;
  delta = sqrt(pow(xAxisGforce - axisXPrevious[sensorNumber], 2) + pow(yAxisGforce - axisYPrevious[sensorNumber], 2) +pow(zAxisGforce - axisZPrevious[sensorNumber], 2));
  
  if (delta > highVibrateFrequency) {
    Serial.println("High Vibration Alert!");
    return "HIGH";
  } 
  else if (delta > midVibrateFrequency) {
    Serial.println("Mid Vibration Alert!");
    return "MID";
  } 
  else if (delta > lowVibrateFrequency) {
    Serial.println("Low Vibration Alert!");
    return "LOW";
  } 
  else {
    Serial.println("No vibration");
    return "NO";
  }
}

String readSoilMoistureSensor() {

}

String readRainSensor() {

}

String checkTilt() {
  groundAcelerometer = readTilt(0);
  midAcelerometer = readTilt(1);
  lowAcelerometer = readTilt(2);

}
//Complete until check titl check vibrate code ok not in checkVibrate() 
//have to add soil moisture , servo motors , Rain sensor , water pump code
String checkVibrate() {
  
}

String checkRain() {

}

String checkSoilMoisture() {

}

void openRoadBarrier() {
  //servo motor open gate
}

void closeRoadBarrier() {
  //servo motor close gate
}

void indicateAlter() {
  //code to activate indicators leds and buzzers when alert
}






