#define RAIN_PIN A0

void setup() {
  Serial.begin(9600);
}

void loop() {
  int raw = analogRead(RAIN_PIN);
  int rainPct = map(raw, 1023, 300, 0, 100);
  rainPct = constrain(rainPct, 0, 100);

  Serial.print("Raw: ");
  Serial.print(raw);
  Serial.print(" | Rain %: ");
  Serial.println(rainPct);

  delay(1000);
}