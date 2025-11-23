#define soilMoistureSensor A0

int moistureRawValue;

int moisturePercentageValue;

int lowPercentageBoundry = 20;
int midPercentageBoundry = 30;
int highPercentageBoundry = 50;

bool lowSoilMoisture;
bool midSoilMoisture;
bool highSoilMoisture;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  moistureRawValue = analogRead(soilMoistureSensor);

  moisturePercentageValue = map(moistureRawValue, 1023, 300, 0, 100);

  moisturePercentageValue = constrain(moisturePercentageValue, 0, 100);

  Serial.println("Raw Value:");
  Serial.println(moistureRawValue);
  Serial.println("Percentage Value:");
  Serial.println(moisturePercentageValue);

  lowSoilMoisture = (moisturePercentageValue > lowPercentageBoundry);

  midSoilMoisture = (moisturePercentageValue > midPercentageBoundry);

  highSoilMoisture = (moisturePercentageValue > highPercentageBoundry);

  if(highSoilMoisture) {

    Serial.println("High Soil Moisture");

  } else if(midSoilMoisture) {

    Serial.println("Mid Soil Moisture");

  } else if(lowSoilMoisture) {

    Serial.println("Low Soil Moisture");

  } else {

    Serial.println("Normal Soil Moisture");

  }
  
  delay(1000);
}
