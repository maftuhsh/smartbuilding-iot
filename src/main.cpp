#include <WiFi.h>
#include "DHTesp.h"
#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

// WIfi 
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// MQTT CONFIG
const char* mqtt_server = "building-iot-9ee940c9.a03.euc1.aws.hivemq.cloud";

const int mqtt_port = 8883;

const char* mqtt_user = "smartbuilding";
const char* mqtt_password = "Smart123";

WiFiClientSecure espClient;
PubSubClient client(espClient);

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
int sudutservoAC;

// lcd OLED Display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

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
void DisplayOLED();
void reconnectMQTT();
void publishSensorData();


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

  // mqtt setup
  espClient.setInsecure();
  client.setServer(mqtt_server, mqtt_port);


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

  // OLED Setup
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED Failed");
    while(true);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(10,20);
  display.println("SMART BUILDING");

  display.display();
  delay(2000);
}

void loop() {

  // mqtt 
  if (!client.connected()) {

    reconnectMQTT();
  }

client.loop();

  Serial.println();
  Serial.println("====================================");
  Serial.println("      SMART BUILDING MONITOR");
  Serial.println("====================================");

  ReadDHT();
  ControlAC();
  ControlMotion();
  ControlLight();
  ControlMQ2();
  DisplayOLED();
  publishSensorData();

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


void DisplayOLED(){

  display.clearDisplay();

  display.setCursor(0,0);
  display.print("MOTION :");
  if (motion == HIGH){
    display.println("DETECTED");
  } 
  else{
    display.println("NO MOTION");
  }

  display.setCursor(0,10);
  display.print("TEMP : ");
  display.print(suhu,1);
  display.println(" C");

  display.setCursor(0,20);
  display.print("HUM  : ");
  display.print(humidity,1);
  display.println(" %");

  display.setCursor(0,30);
  display.print("LIGHT: ");
  
  if(Cahaya > 1500){
    display.println("DARK");
  } else{
    display.println("BRIGHT");
  }

  display.setCursor(0,40);
  display.print("GAS  : ");

  if(smoke < 1800){
    display.println("SAFE");
  }
  else if(smoke < 3000){
    display.println("WARNING");
  }
  else{
    display.println("DANGER");
  }



  display.display();
}


void reconnectMQTT() {

  while (!client.connected()) {

    Serial.print("Connecting MQTT...");

    String clientId = "ESP32-" + String((uint32_t)ESP.getEfuseMac(), HEX);

    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {

      Serial.println(" CONNECTED");
      Serial.print("MQTT STATE: ");
      Serial.println(client.state());

      // optional debug info
      Serial.println("MQTT ready to publish/subscribe");

      break; // penting: keluar dari loop kalau sudah connect

    } else {

      Serial.print(" FAILED, rc=");
      Serial.println(client.state());

      Serial.println("Retry in 2 seconds...");

      delay(2000);
    }
  }
}


void publishSensorData() {

  if (!client.connected()) {
    Serial.println("MQTT NOT CONNECTED → skip publish");
    return;
  }

  String payload = "{";

  payload += "\"temp\":" + String(suhu) + ",";
  payload += "\"hum\":" + String(humidity) + ",";
  payload += "\"motion\":" + String(motion) + ",";
  payload += "\"light\":" + String(Cahaya) + ",";
  payload += "\"gas\":" + String(smoke);

  payload += "}";

  Serial.print("Publish: ");
  Serial.println(payload);

  bool success = client.publish("smartbuilding/sensor", payload.c_str());

  if (success) {
    Serial.println("Publish SUCCESS");
  } else {
    Serial.println("Publish FAILED");
  }
}