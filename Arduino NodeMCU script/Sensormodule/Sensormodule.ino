#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <sps30.h>
#include <SensirionI2CSvm41.h>
#include <SensirionI2CScd4x.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>
#include "connection_secret.h"

// Define MQTT max packet size
#define MQTT_MAX_PACKET_SIZE 256

// Wi-Fi credentials
const char* ssid = NET_SSID;
const char* password = NET_PASSWORD;

// MQTT broker details
const char* mqtt_server = "192.168.0.105";
const int mqtt_port = 1883;
const char* sensor_topic1 = "home/sensor1";

//Wifi and MQTT clients
WiFiClient wifi;
PubSubClient client(wifi);

// Function to connect to WiFi
void setup_wifi(){
  delay(10);
  Serial.println();
  Serial.print("Connecting to... ");
  Serial.println(ssid);

  WiFi.begin(ssid , password);

  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print("...");
  }

  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Function to reconnect to MQTT broker
void reconnect(){
  while (!client.connected()) {
    Serial.println("Attempting to connect to MQTT client...");
    if(client.connect("ESP8266Client")){
      Serial.println("Connected");
    }
    else{
      Serial.print("failed, rc = ");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds...");
      delay(5000);
    }
  }
}

SensirionI2CScd4x scd4x;
SensirionI2CSvm41 svm41;
Adafruit_LIS3DH lis = Adafruit_LIS3DH();

unsigned long previousmillis = 0;
const unsigned long interval = 15 * 60 * 1000 ; //15 minutes delay

void setup() {
  uint16_t error_svm41 ;
  uint16_t error_scd41 ;
  uint16_t error_sps30 ;
  char errorMessage[256];
  

  Serial.begin(115200);
  setup_wifi();
  Wire.begin();
  client.setServer(mqtt_server, mqtt_port);
  svm41.begin(Wire);
  scd4x.begin(Wire);
  sensirion_i2c_init();

  // Start Measurement for
  error_svm41 = svm41.startMeasurement();
  if(error_svm41){
    Serial.print("Error trying to execute Measurement() : ");
    errorToString( error_svm41, errorMessage, 256);
    Serial.println(errorMessage);
  }

  error_scd41 = scd4x.startPeriodicMeasurement();
  if(error_scd41){
    Serial.print("Error trying to execute startPerodicMeasurement() : ");
    errorToString(error_scd41, errorMessage, 256);
    Serial.println(errorMessage);
  }

  error_sps30 = sps30_start_measurement();
  if(error_sps30 < 0){
    Serial.println("Error in starting SPS30 Measurement");
  }

  //start measurement for LIS3DH
   if (! lis.begin(0x18)) {   // change this to 0x19 for alternative i2c address
    Serial.println("Couldnt start");
    while (1) yield();
  }
  Serial.println("LIS3DH found!");
  lis.setRange(LIS3DH_RANGE_4_G); 
  lis.setPerformanceMode(LIS3DH_MODE_HIGH_RESOLUTION);
  lis.setDataRate(LIS3DH_DATARATE_400_HZ);

}

void loop() {
  if (!client.connected()){
    reconnect();
  }
  client.loop();

  unsigned long currentmillis = millis();
  if (currentmillis - previousmillis >= interval){

    previousmillis = currentmillis ;
  
    // Allocate a JSON document
    JsonDocument doc1;

    // Create the payload
    char output1[256];

    // start reading measurements
    float humidity_svm41;
    float temperature_svm41;
    float vocIndex;
    float noxIndex;
    uint16_t co2 = 0;
    float temperature = 0.0f;
    float humidity = 0.0f;
    int sound;

    //read noise
    analogRead(0);
    sound = analogRead(0);
    Serial.print("Sound :");
    Serial.print(sound);
    Serial.println();
    
    uint16_t svm41_read;
    svm41_read = svm41.readMeasuredValues(humidity_svm41, temperature_svm41, vocIndex,noxIndex);
    Serial.print("Humidity:");
    Serial.print(humidity_svm41);
    Serial.print("\t");
    Serial.print("Temperature:");
    Serial.print(temperature_svm41);
    Serial.print("\t");
    Serial.print("VocIndex:");
    Serial.print(vocIndex);
    Serial.print("\t");
    Serial.print("NoxIndex:");
    Serial.println(noxIndex);

    uint16_t scd41_read;
    scd41_read = scd4x.readMeasurement(co2, temperature, humidity);
    Serial.print("Co2:");
    Serial.print(co2);
    Serial.println();

    struct sps30_measurement sps_30;
    uint16_t read_sps30;
    read_sps30 = sps30_read_measurement(&sps_30);
    Serial.print("Typical particle size: ");
    Serial.println(sps_30.typical_particle_size);
    //Serial.println();

    sensors_event_t event;
    lis.getEvent(&event);
    float acc_x = event.acceleration.x ;
    float acc_y = event.acceleration.y ;
    float acc_z = event.acceleration.z ;
    float vibrations = sqrt(acc_x * acc_x + acc_y * acc_y + acc_z * acc_z);

    /* Display the results (acceleration is measured in m/s^2) */
    Serial.print("X: "); Serial.print(event.acceleration.x);
    Serial.print("\tY: "); Serial.print(event.acceleration.y);
    Serial.print("\tZ: "); Serial.print(event.acceleration.z);
    Serial.println(" m/s^2 ");
    Serial.print("Vibration: "); Serial.print(vibrations);
    Serial.println(" m/s^2 ");
    Serial.println();

    // Poplulate JSON document
    doc1["temp"] = round(temperature_svm41*100.0)/100.0;
    doc1["hum"] = round(humidity_svm41*100.0)/100.0;
    doc1["voc"] = vocIndex;
    doc1["nox"] = noxIndex;
    doc1["co2"] = co2;
    doc1["sps30"] = round(sps_30.typical_particle_size*100.0)/100.0;
    doc1["sound"] = sound;
    doc1["vib"] = round(vibrations*100.0)/100.0;

    serializeJson(doc1, output1, sizeof(output1));

    Serial.println(output1); 

    if (client.publish(sensor_topic1, output1)) {
          Serial.println("Data sent.");
          Serial.println();
    } else {
          Serial.println("Failed to send Data");
    }
  }

}
