// ---------- Arduino UNO code (Soil Moisture) ----------

const int SOIL_PIN = A0;

// Calibration values
int dryValue  = 1023;  // completely dry
int wetValue  = 300;   // very wet

int readSoilPercent() {
  int raw = analogRead(SOIL_PIN);

  int percent = map(raw, dryValue, wetValue, 0, 100);
  percent = constrain(percent, 0, 100);

  return percent;
}

void setup() {
  Serial.begin(9600);   // communication to ESP8266
}

void loop() {
  int soilPercent = readSoilPercent();
  Serial.println(soilPercent);  // send to ESP8266

  delay(2000); // update every 2 seconds
}
