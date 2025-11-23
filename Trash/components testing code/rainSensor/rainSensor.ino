#define rainSensorPin A0

int rawValue;
int rainPercent;

float lowRainPercent = 20;  // example threshold
float midRainPercent = 50;  // example threshold
float highRainPercent = 80; // example threshold

bool lowRain;
bool midRain;
bool highRain;

void setup() {
  Serial.begin(9600);
}

void loop() {

  rawValue = analogRead(rainSensorPin);

  // Convert sensor value to percentage
  rainPercent = map(rawValue, 1023, 300, 0, 100);
  rainPercent = constrain(rainPercent, 0, 100);

  Serial.print("Rain Percentage: ");
  Serial.println(rainPercent);

  // Compare with thresholds
  lowRain = (rainPercent > lowRainPercent);
  midRain = (rainPercent > midRainPercent);
  highRain = (rainPercent > highRainPercent);

  if (highRain) {
    Serial.println("HIGH RAIN ALERT");
  } 
  else if (midRain) {
    Serial.println("MEDIUM RAIN ALERT");
  } 
  else if (lowRain) {
    Serial.println("LOW RAIN ALERT");
  } 
  else {
    Serial.println("No Rain");
  }

  delay(1000);
}
