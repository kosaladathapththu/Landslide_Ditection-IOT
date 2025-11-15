// ----- Pin Setup -----
int moisturePin = 34;      // Analog pin for soil moisture
int tiltPin = 27;          // Digital input for tilt
int vibPin = 26;           // Digital input for vibration

int warnLED = 14;          // Yellow LED
int dangerLED = 12;        // Red LED
int buzzer = 13;           // Buzzer

void setup() {
  Serial.begin(115200);

  pinMode(tiltPin, INPUT);
  pinMode(vibPin, INPUT);
  pinMode(warnLED, OUTPUT);
  pinMode(dangerLED, OUTPUT);
  pinMode(buzzer, OUTPUT);

  Serial.println("Starting Sensor Test…");
}

void loop() {
  int moistureValue = analogRead(moisturePin); // 0–4095 on ESP32
  int tiltState = digitalRead(tiltPin);
  int vibState = digitalRead(vibPin);

  Serial.print("Moisture: ");
  Serial.print(moistureValue);
  Serial.print(" | Tilt: ");
  Serial.print(tiltState);
  Serial.print(" | Vibration: ");
  Serial.println(vibState);

  // ----- LED + BUZZER test logic -----
  if (moistureValue > 3000) {
    digitalWrite(warnLED, HIGH);
  } else {
    digitalWrite(warnLED, LOW);
  }

  if (tiltState == HIGH || vibState == HIGH) {
    digitalWrite(dangerLED, HIGH);
    digitalWrite(buzzer, HIGH);
  } else {
    digitalWrite(dangerLED, LOW);
    digitalWrite(buzzer, LOW);
  }

  delay(300);
}
