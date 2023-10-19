#include<Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

// Update these with values suitable for your network.

const char* SSID = "Khánh Duy";
const char* PASSWORD = "0982104532";
const char* mqtt_server = "broker.emqx.io";
const char* clientId = "AirConditionerNode";
const int mqtt_port = 1883;

#define DHT_Pin 2
#define LDR_Pin PIN_A0
#define DHTTYPE DHT11

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHT_Pin, DHTTYPE);


int brightness = 0;
float dataReceive = 0;
float temp = 0;

void SetupWifi();
void ConnectToBroker();
void InitMQTTProtocol();
void MQTTCallback(char* , byte* , unsigned int);


void SetupWifi() {
  Serial.print("Connecting to ");
  Serial.println(SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void ConnectToBroker() {
  while (!client.connected()) {
    Serial.print("Initialize MQTT connection...");
    if (client.connect(CLIENT_ID)) {
      Serial.println("The MQTT broker is connected");
      client.subscribe(CTRL_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void InitMQTTProtocol()
{
  SetupWifi();
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(MQTTCallback);
  ConnectToBroker();
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < (int)length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  payload[length] = '\0';
    dataReceive = String((char*) payload).toFloat();

    Serial.println(topic);
    //Serial.println(value);

}

void MQTTCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < (int)length; i++) {
    Serial.print((char)payload[i]);
  }
  payload[length] = '\0';
  char* message = (char*) payload;
  if (strcmp(topic, CTRL_TOPIC) == 0) {
    controlMessage = atoi(message);
  }
  if (strcmp(topic, TEMP_TOPIC) == 0) {
    temperature = atof(message);
  }
  if (strcmp(topic, IN_LDR_TOPIC) == 0) {
    indoorLight = atoi(message);
  }
}

void setup() {
  Serial.begin(115200);
  InitMQTTProtocol();
  dht.begin();
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  temp = dht.readTemperature();
  brightness = analogRead(LDR_Pin);

  client.publish("SmartHome/Temperature", String(temp).c_str());
  client.publish("SmartHome/Brightness", String(brightness).c_str());
  delay(5000);
  
}