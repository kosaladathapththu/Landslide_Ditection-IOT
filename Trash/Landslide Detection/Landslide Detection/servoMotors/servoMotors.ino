#include <ESP8266Servo.h>

Servo servo1;
Servo servo2;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  servo1.attach(D5);
  servo2.attach(D6);
  
  servo1.write(0);
  servo2.write(0);

  delay(1000);
}

void openGate(){
  servo1.write(0);
  servo2.write(0);
}

void closeGate(){
  servo1.write(90);
  servo2.write(90);
}

void loop() {
  // put your main code here, to run repeatedly:
  closeGate();
  delay(2000);
  openGate();
  delay(2000);

}
