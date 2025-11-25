#include <DHT.h>

#define DHTPIN 7
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  dht.begin();
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("DHT Error!");
    return;
  }

  Serial.print("Temp: ");
  Serial.print(t);
  Serial.print("C  | Humidity: ");
  Serial.print(h);
  Serial.println("%");

  delay(2000);
}

