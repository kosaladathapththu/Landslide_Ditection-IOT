#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <FirebaseJson.h>
#include <SoftwareSerial.h>

#define WIFI_SSID "ZTE Blade V50 Design"
#define WIFI_PASSWORD "imaadh0234897651"

#define FIREBASE_URL "https://iot-project-3c897-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define FIREBASE_KEY "AIzaSyCSEWW0wawOzJgvxyOmDqXMqE8nihprCd0"

#define SERVO1_PIN D1
#define SERVO2_PIN D2

SoftwareSerial arduinoSerial(D5,D6); // RX,TX to Arduino
FirebaseData fbdo;
FirebaseJson json;

void setup(){
  Serial.begin(9600);
  arduinoSerial.begin(9600);

  pinMode(SERVO1_PIN,OUTPUT);
  pinMode(SERVO2_PIN,OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while(WiFi.status()!=WL_CONNECTED){delay(300);}
  Firebase.begin(FIREBASE_URL,FIREBASE_KEY);
  Firebase.reconnectWiFi(true);
}

void loop(){
  if(arduinoSerial.available()){
    String data = arduinoSerial.readStringUntil('\n');
    data.trim();
    if(data.length()==0) return;

    float values[10];
    int start=0, idx=0;
    while(idx<10){
      int comma=data.indexOf(',',start);
      if(comma==-1) comma=data.length();
      values[idx++] = data.substring(start,comma).toFloat();
      start=comma+1;
    }

    json.clear();
    json.set("tilt1",values[0]);
    json.set("tilt2",values[1]);
    json.set("tilt3",values[2]);
    json.set("vib1",values[3]);
    json.set("vib2",values[4]);
    json.set("vib3",values[5]);
    json.set("soil",values[6]);
    json.set("rain",values[7]);
    json.set("humidity",values[8]);
    json.set("temp",values[9]);

    Firebase.RTDB.setJSON(&fbdo,"/Landslide/Device1",&json);
  }

  // Gate Control from Firebase
  if(Firebase.RTDB.getString(&fbdo,"/Landslide/Device1/GateControl")){
    String cmd=fbdo.stringData();
    if(cmd=="OPEN"){digitalWrite(SERVO1_PIN,LOW); digitalWrite(SERVO2_PIN,LOW);}
    else if(cmd=="CLOSE"){digitalWrite(SERVO1_PIN,HIGH); digitalWrite(SERVO2_PIN,HIGH);}
  }

  delay(1000);
}
