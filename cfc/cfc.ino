#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "DHT.h"

// --------- WiFi ----------
#define WIFI_SSID "ZTE Blade V50 Design"
#define WIFI_PASSWORD "imaadh0234897651"

// --------- Firebase ----------
#define API_KEY "AIzaSyCSEWW0wawOzJgvxyOmDqXMqE8nihprCd0"
#define DATABASE_URL "https://iot-project-3c897-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_EMAIL "rushdeeimaadh@gmail.com"
#define FIREBASE_PASSWORD "ir@02348"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseJson json;

// --------- DHT22 ----------
#define DHTPIN D5     // Choose any GPIO pin
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  dht.begin();

  // Connect to WiFi
  Serial.print("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");

  // Setup Firebase
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = FIREBASE_EMAIL;
  auth.user.password = FIREBASE_PASSWORD;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("Firebase ready!");
}

void loop() {
  float temperature = dht.readTemperature(); // Celsius
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    Serial.printf("Temp: %.2f C, Humidity: %.2f %%\n", temperature, humidity);

    // Send to Firebase
    json.clear();
    json.set("temperature", temperature);
    json.set("humidity", humidity);

    if (Firebase.RTDB.setJSON(&fbdo, "/Landslide/Device1/DHT22", &json)) {
      Serial.println("Firebase upload OK");
    } else {
      Serial.println("Firebase error: " + fbdo.errorReason());
    }
  }

  delay(3000); // Update every 3 seconds
}
