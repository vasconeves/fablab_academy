/*
  MODIFIED BY VASCO NEVES @ 09/05/2021
  
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/cloud-mqtt-mosquitto-broker-access-anywhere-digital-ocean/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <WiFi.h>
extern "C" {
  #include "freertos/FreeRTOS.h" //including h files from the ESP32 library
  #include "freertos/timers.h"   //
}
#include <AsyncMqttClient.h>

#define WIFI_SSID "baleinha voadora" //local WIFI SSID
#define WIFI_PASSWORD "123456789" //local WIFI PASSWORD

// Digital Ocean MQTT Mosquitto Broker
#define MQTT_HOST IPAddress(104, 248, 192, 136) //MQTT IP
// For a cloud MQTT broker, type the domain name
//#define MQTT_HOST "example.com"
#define MQTT_PORT 1883 // using the default port 1883

#define MQTT_USERNAME "vneves"
#define MQTT_PASSWORD "oratoroeuarolha"

// Test MQTT Topic
#define MQTT_PUB_TEST "test"

AsyncMqttClient mqttClient; //Declaring the AsyncMqttClient as mqttclient
TimerHandle_t mqttReconnectTimer; //Timer handles for the mqtt and wifi reconnection
TimerHandle_t wifiReconnectTimer;

unsigned long previousMillis = 0;   // Stores last time information was published
const long interval = 5000;         // Interval at which to publish information

int i = 0; //declration of a counter for the number of published packets

// subfunctions
//function to connecto to the wifi network
void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}
//function to connect to the MQTT server
void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}
//function to flag WiFi encoded events from 0 to 25 (check text)
void WiFiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch(event) { //switch...case control structure
    case SYSTEM_EVENT_STA_GOT_IP: //this is event 7
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      connectToMqtt();
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED: //this is event 5
      Serial.println("WiFi lost connection");
      xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
      xTimerStart(wifiReconnectTimer, 0);
      break;
  }
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");
  if (WiFi.isConnected()) {
    xTimerStart(mqttReconnectTimer, 0);
  }
}

/*void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}
void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}*/

void onMqttPublish(uint16_t packetId) {
  Serial.print("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt)); //xtimercreate is a function from the freeRTOS library
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi)); //check the main text

  WiFi.onEvent(WiFiEvent); //if there is a Wi-fi event the program automatically calls the function WifiEvent. This has global reach.

  mqttClient.onConnect(onMqttConnect); //the function onMqttConnect is called if there is an MQTT connection
  mqttClient.onDisconnect(onMqttDisconnect); //the function onMqttDisconnect is called if there is an MQTT disconnecton
  /*mqttClient.onSubscribe(onMqttSubscribe); //these two lines are only used if the ESP32 is going to subscribe as well as to publish
  mqttClient.onUnsubscribe(onMqttUnsubscribe);*/
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT); //MTTT server configuration
  // If your broker requires authentication (username and password), set them below
  mqttClient.setCredentials(MQTT_USERNAME, MQTT_PASSWORD); 
  connectToWifi(); //The wifi connection is established (or not)
}

void loop() {
  unsigned long currentMillis = millis();
  // Every X number of seconds (interval = 5 seconds) 
  // it publishes a new MQTT message
  if (currentMillis - previousMillis >= interval) {
    // Save the last time a new reading was published
    previousMillis = currentMillis;
    
    String testString = "Hello, world! #" + String(i);
    // Publish an MQTT message on topic test
    uint16_t packetIdPub1 = mqttClient.publish(MQTT_PUB_TEST, 1, true, String(testString).c_str());                            
    Serial.printf("Publishing on topic %s at QoS 1, packetId: %i", MQTT_PUB_TEST, packetIdPub1);
    Serial.print(" Message: ");
    Serial.println(testString);
    i++;
  }
}
