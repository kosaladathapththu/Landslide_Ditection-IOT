// Soil Moisture Sensor Test (ESP8266 NodeMCU)
#define SOIL_PIN A0

void setup() {
  Serial.begin(9600);
}

void loop() {
  int raw = analogRead(SOIL_PIN);
  int moisture = map(raw, 1023, 300, 0, 100);
  moisture = constrain(moisture, 0, 100);

  Serial.print("Raw: ");
  Serial.print(raw);
  Serial.print("  Moisture %: ");
  Serial.println(moisture);

  delay(1000);
}
