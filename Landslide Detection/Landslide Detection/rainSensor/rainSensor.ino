#define rainSensorPin A0

int rawValue;
int rainPercentage;

float lowRainPercent;
float midRainPercent;
float highRainPercent;

bool lowRain;
bool midRain;
bool highRain;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  rawValue = analogRead(rainSensorPin);

  rainPercent = map(rawValue, 1023, 300, 0, 100);
  rainPercent = constraint(rainPercent, 0, 100);

  lowRain = (rainPercent > lowRainPercent);
  midRain = (rainPercent > midRainPercent);
  highRain = (rainPercent > highRainPercent);

  if(highRain) {

    Serial.println("HIGH RAIN ALTER");

  } else if(midRain) {

    Serial.println("HIGH RAIN ALTER");

  } else if(lowRain) {

    Serial.println("HIGH RAIN ALTER");

  } else {
    
    Serial.println("No Rain");
  
  }

}
