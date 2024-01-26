#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <ArduinoMqttClient.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <time.h>
#include "arduino_secrets.h"
#include <Arduino_ConnectionHandler.h>
#include <arduino-timer.h>

WiFiConnectionHandler conMgr(SECRET_SSID, SECRET_PASS);
StaticJsonDocument<200> doc;
auto timer = timer_create_default(); // create a timer with default settings

// NTP Time 
WiFiUDP NTPUdp;
NTPClient timeClient(NTPUdp,"pool.ntp.org");

// Clients
WiFiClient wifiClient;
BearSSLClient ssl_client(wifiClient);
MqttClient mqtt_client(ssl_client);

void setup() {
  Serial.begin(115200);
  while(!Serial) {
    delay(1000);
  }

  if (!ECCX08.begin()) {
      Serial.println("No ECCX08 present!");
      while (1);
  }
  pinMode(LED_BUILTIN, OUTPUT); // set LED pin to OUTPUT
    
  Serial.println("Using BEARSSL CLIENT");
  ArduinoBearSSL.onGetTime(getTime);
  ssl_client.setKey(SECRET_PRIKEY, SECRET_DEVCERT);
  
  timeClient.begin();
  
  // Init MQTT client
  mqtt_client.setId(SECRET_CLIENT_ID);
  mqtt_client.onMessage(onMQTTMessage);
  
  // Connection Manager callbacks
  conMgr.addCallback(NetworkConnectionEvent::CONNECTED, onNetworkConnect);
  conMgr.addCallback(NetworkConnectionEvent::DISCONNECTED, onNetworkDisconnect);
  conMgr.addCallback(NetworkConnectionEvent::ERROR, onNetworkError);
    
  // JSON Payload formatting
  // Refer to the IoTConnect D2C documentation for formatting 
  JsonObject data = doc["d"][0].createNestedObject("d");      
  data["Temperature"] = 0;
  data["text"] = "HELLO FROM H7!!";

  timer.every(20000, publishData);  
}

void loop() {
  mqtt_client.poll();
  timer.tick(); 
  const auto conStatus = conMgr.check();

  if (conStatus == NetworkConnectionState::CONNECTED){
    timeClient.update();
    if (!mqtt_client.connected()) {    
      connectMQTT();
    }
  }
  else{
    // Serial.println("NetworkConnectionState not Connected");
    ;
  }
}

 // =================== Networking Handlers ======================================

void onNetworkConnect() {
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  Serial.println();
  Serial.println("Fetching network time");
  timeClient.forceUpdate();

  unsigned long mytime = getTime();
  Serial.print("Time set to: ");
  Serial.println(mytime);
}


void onNetworkDisconnect(){
  Serial.println(">>>> DISCONNECTED from network <<<<<");
}


void onNetworkError(){
   Serial.println(">>>> NETWORK ERROR <<<<");
}

 // =================== MQTT Handlers ======================================

void connectMQTT(){
  
//  while (!mqtt_client.connect(SECRET_BROKER, 8883)) {
  if (!mqtt_client.connect(SECRET_BROKER, 8883)) {
      //Serial.print(".");
      Serial.print("MQTT Error= ");
      Serial.println(mqtt_client.connectError());
      Serial.print("SSL Error= ");
      Serial.println(ssl_client.errorCode());
      Serial.println(getTime());
      delay(3000);
  }
  else {
    Serial.println(" MQTT connected!");
    // subscribe topic
    mqtt_client.subscribe(SECRET_CMD_TOPIC);
  }
}


void onMQTTMessage(int messageSize){
   // we received a message, print out the topic and contents
  Serial.println();
  Serial.print("Received ");
  Serial.print(messageSize);
  Serial.print(" bytes from topic: ");
  Serial.print(mqtt_client.messageTopic());
  Serial.print(" and payload:  '");
  while (mqtt_client.available()) {
    Serial.print((char)mqtt_client.read());
  }
  Serial.println();
}


bool publishData(void *) {
  if(mqtt_client.connected()) {
    // Post data to IoTCOnnect
    Serial.print("Posting msg to topic: ");
    doc["d"][0]["d"]["Temperature"] = random(1, 212);
    doc["d"][0]["d"]["text"] = timeClient.getFormattedTime();
    serializeJson(doc, Serial);
    Serial.println();
    mqtt_client.beginMessage(SECRET_REP_TOPIC);
    serializeJson(doc, mqtt_client);
    mqtt_client.endMessage();
    return true;
 }
 else
  return false;
}


bool toggle_led(void *) {
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // toggle the LED
  return true; // repeat? true
}

unsigned long getTime() {
  return timeClient.getEpochTime();
}
