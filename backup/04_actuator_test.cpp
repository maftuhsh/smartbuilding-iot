#include <WiFi.h>
#include "DHTesp.h"
#include <sensor.h>
#include <ESP32Servo.h>

// WIfi 
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Sensor PIN
const int DHT_PIN = 15;
const int PIR_PIN = 5;
const int LDR_PIN = 34;
const int MQ2_PIN = 35;

// actuator
const int LAMP_ALRM = 19;
const int LAMP_TERAS_PIN = 18;
const int LAMP_RUANGAN_PIN = 13;
const int SERVO_AC = 23;

// Global variabel Sensor
float suhu;
float humidity;
int smoke;
int Cahaya;
bool motion;

// Dht 
DHTesp dhtSensor;

// servo
Servo servoAC;

// Function Declaration
void ReadDHT();
void ControlAC();
void ControlMotion();
void ControlLight();
void ControlMQ2();

void setup() {

  Serial.begin(115200);

  Serial.print("Connecting WiFi");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());


  // dht
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);

  // serco ac
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  servoAC.setPeriodHertz(50); // wajib untuk servo
  servoAC.attach(SERVO_AC, 500, 2400);
  servoAC.write(0); // nilai awal setup

  // pir sebagai input
  pinMode(PIR_PIN, INPUT);
  // led sebagau ouput
  pinMode(LAMP_TERAS_PIN, OUTPUT);

  // led LDR
  pinMode(LAMP_RUANGAN_PIN,OUTPUT);

  // buzzer setup
  pinMode(LAMP_ALRM, OUTPUT);
}

void loop() {

  Serial.println();
  Serial.println("====================================");
  Serial.println("      SMART BUILDING MONITOR");
  Serial.println("====================================");

  ReadDHT();
  ControlAC();
  ControlMotion();
  ControlLight();
  ControlMQ2();

  Serial.println("====================================");
  delay(1000);

}


// Function Content

void ReadDHT(){

  TempAndHumidity data = dhtSensor.getTempAndHumidity();

  suhu = data.temperature;
  humidity = data.humidity;

  Serial.print("TEMP     : ");
  Serial.print(suhu,1);
  Serial.println(" C");

  Serial.print("HUMIDITY : ");
  Serial.print(humidity,1);
  Serial.println(" %");

}

void ControlAC(){
  // ambil nilai kerja dari sensor

  // logika bukaan ac
  int sudutservoAC;

  if (suhu < 20) {
    sudutservoAC = 0;
  }
  else if (suhu >= 20 && suhu < 25) {
    sudutservoAC = 60;
  }
  else if (suhu >= 25 && suhu < 30) {
    sudutservoAC = 120;
  }
  else {
    sudutservoAC = 180;
  }

  Serial.print("AC SERVO : ");
  Serial.println(sudutservoAC);

  servoAC.write(sudutservoAC);
}

void ControlMotion(){
    //PIR
  motion = digitalRead(PIR_PIN);
  if (motion == HIGH){
    Serial.println("MOTION   : DETECTED");
    digitalWrite(LAMP_TERAS_PIN,HIGH);

  } else{
    Serial.println("MOTION   : NO MOTION");
    digitalWrite(LAMP_TERAS_PIN,LOW);
  }

}

void ControlLight(){
    // LDR
  Cahaya = analogRead(LDR_PIN);


  Serial.print("LIGHT    : ");
  Serial.print(Cahaya);

  if(Cahaya > 1500){
    Serial.println(" | DARK");
    digitalWrite(LAMP_RUANGAN_PIN,HIGH);
  }else{
    Serial.println(" | BRIGHT");
    digitalWrite(LAMP_RUANGAN_PIN,LOW);
  }

}

void ControlMQ2(){
  // MQ2
  smoke = analogRead(MQ2_PIN);
  
  Serial.print("GAS      : ");
  Serial.print(smoke);


  if (smoke < 1800){
    Serial.println(" | SAFE");
    digitalWrite(LAMP_ALRM,LOW);
  }
  else if(smoke >= 1800 && smoke < 3000){

    Serial.println(" | WARNING");
    digitalWrite(LAMP_ALRM,HIGH);
    delay(500);

    digitalWrite(LAMP_ALRM,LOW);
    delay(500);
  }
  else{
    Serial.println(" | DANGER");

    digitalWrite(LAMP_ALRM,HIGH);
    delay(100);

    digitalWrite(LAMP_ALRM,LOW);
    delay(100);
  }

}


