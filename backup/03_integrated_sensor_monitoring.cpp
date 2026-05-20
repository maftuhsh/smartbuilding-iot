#include "DHTesp.h"

const int DHT_PIN = 15;
const int PIR_PIN = 5;
const int LDR_PIN = 34;
const int MQ2_PIN = 35;




DHTesp dhtSensor;

void setup() {
  Serial.begin(115200);
  // dht
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);
  // pir
  pinMode(PIR_PIN, INPUT);
}

void loop() {
  // DHT
  TempAndHumidity  data = dhtSensor.getTempAndHumidity();
  Serial.println("Temp: " + String(data.temperature, 2) + "°C");
  Serial.println("Humidity: " + String(data.humidity, 1) + "%");

  //PIR
  int motion = digitalRead(PIR_PIN);
  if (motion == HIGH){
    Serial.println("Motion: Detected");
  } else{
    Serial.println("Motion: No Motion");
  }
  Serial.println("---");

  // LDR
  int Cahaya = analogRead(LDR_PIN);
  Serial.println("LIGHT: "+ String(Cahaya));

  // Mq2
  int smoke = analogRead(MQ2_PIN);
  Serial.println("GAS: " + String(smoke));

  delay(2000); // Wait for a new reading from the sensor (DHT22 has ~0.5Hz sample rate)
}