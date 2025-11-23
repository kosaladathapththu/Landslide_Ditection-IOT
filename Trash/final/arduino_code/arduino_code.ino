#include <Wire.h>
#include <MPU6050.h>
#include <DHT.h>

#define DHT_PIN 7
#define DHT_TYPE DHT22
#define SOIL_PIN A0
#define RAIN_PIN A1
#define BUZZER_PIN 8
#define LED_PIN 9
#define SERVO1_PIN 10
#define SERVO2_PIN 11

MPU6050 mpu1(0x68);
MPU6050 mpu2(0x69);
MPU6050 mpu3(0x70); // if address changed
DHT dht(DHT_PIN, DHT_TYPE);

float prevPitch[3] = {0,0,0};
float vibThresholdLow = 50; // adjust
float vibThresholdMid = 150;
float vibThresholdHigh = 300;

// RAW readings (from MPU sensors)
int16_t rawAx[3], rawAy[3], rawAz[3];



void setup() {
  Serial.begin(9600);
  Wire.begin();
  dht.begin();
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  float ax[3], ay[3], az[3], pitch[3], vibration[3];

  // Read MPU6050
  MPU6050* mpus[3] = {&mpu1,&mpu2,&mpu3};
  for(int i=0;i<3;i++){
    mpus[i]->getAcceleration(&rawAx[i], &rawAy[i], &rawAz[i]);

// Convert to 'g' (gravity units)
ax[i] = rawAx[i] / 16384.0;
ay[i] = rawAy[i] / 16384.0;
az[i] = rawAz[i] / 16384.0;

  }

  // Soil moisture
  float soilRaw = analogRead(SOIL_PIN);
  float soilPercent = map(soilRaw, 0, 1023, 0, 100);

  // Rain sensor
  float rainRaw = analogRead(RAIN_PIN);
  float rainMM = map(rainRaw, 0, 1023, 0, 200); // approximate mm/day

  // Temp/Humidity
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  // Risk logic example (simplified, adjust thresholds)
  String tiltRisk="", soilRisk="", rainRisk="";
  for(int i=0;i<3;i++){
    if(abs(pitch[i])<15) tiltRisk="Very Low";
    else if(abs(pitch[i])<25) tiltRisk="Low-Moderate";
    else if(abs(pitch[i])<35) tiltRisk="Moderate-High";
    else if(abs(pitch[i])<45) tiltRisk="High";
    else tiltRisk="Critical";
  }
  if(soilPercent<20) soilRisk="Very Low";
  else if(soilPercent<40 && abs(pitch[0])>25) soilRisk="Moderate";
  else if(soilPercent<60 && abs(pitch[0])>25) soilRisk="High";
  else if(soilPercent>60 && abs(pitch[0])>15) soilRisk="Critical";

  if(rainMM>=150) rainRisk="High";
  else if(rainMM>=100) rainRisk="Moderate";
  else if(rainMM>=75) rainRisk="Low";
  else rainRisk="Very Low";

  // Local Alerts
  if(soilRisk=="Critical" || tiltRisk=="Critical" || rainRisk=="High"){
    digitalWrite(BUZZER_PIN,HIGH);
    digitalWrite(LED_PIN,HIGH);
  } else {
    digitalWrite(BUZZER_PIN,LOW);
    digitalWrite(LED_PIN,LOW);
  }

  // Send CSV to ESP
  Serial.print(pitch[0]); Serial.print(",");
  Serial.print(pitch[1]); Serial.print(",");
  Serial.print(pitch[2]); Serial.print(",");
  Serial.print(vibration[0]); Serial.print(",");
  Serial.print(vibration[1]); Serial.print(",");
  Serial.print(vibration[2]); Serial.print(",");
  Serial.print(soilPercent); Serial.print(",");
  Serial.print(rainMM); Serial.print(",");
  Serial.print(hum); Serial.print(",");
  Serial.println(temp);

  delay(1000);
}
