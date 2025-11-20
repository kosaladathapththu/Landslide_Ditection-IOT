#include <ESP8266WiFi.h>
#include <Servo.h>

// -------- Servo Pins ----------
#define SERVO1_PIN D3
#define SERVO2_PIN D4

Servo servo1;
Servo servo2;

// -------- Gate Positions ----------
int gateUp = 0;     // Adjust depending on your servo mounting
int gateDown = 90;  // Servo angle when gate is down

void setup() {
  Serial.begin(9600);

  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);

  // Start with gates down
  closeGates();
}

void loop() {
  // Example: open gates, wait, then close
  openGates();
  delay(3000); // gates stay up for 3 seconds
  closeGates();
  delay(3000);
}

// -------- Methods --------

// Open both gates simultaneously
void openGates() {
  servo1.write(gateUp);       // Servo1 goes UP
  servo2.write(180 - gateUp); // Servo2 faces opposite, so invert angle
  Serial.println("Gates opening...");
}

// Close both gates simultaneously
void closeGates() {
  servo1.write(gateDown);       // Servo1 goes DOWN
  servo2.write(180 - gateDown); // Servo2 faces opposite
  Serial.println("Gates closing...");
}
