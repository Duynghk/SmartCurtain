#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include "config.h"

#define SENSOR_READING_INTERVAL 1000
#define DHT_Pin 2
#define LDR_Pin PIN_A0
#define DHTTYPE DHT11

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHT_Pin, DHTTYPE);
os_timer_t timerInterrupt;

int controlMessage = OFF;
float temperature = 0;
float humidity = 0;
int indoorLight = 0;
float lastTemp = 0;
float lastHumi = 0;
int lastLight = 0;


void SetupWifi();
void ConnectToBroker();
void InitMQTTProtocol();
void MQTTCallback(char* , byte* , unsigned int);
void TimerCallback(void *);

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
    if(strcmp(message, "ACNodeOn") == 0) 
    {
      os_timer_arm(&timerInterrupt, SENSOR_READING_INTERVAL, true);
    }
    if(strcmp(message, "ACNodeOff") == 0)
    {
      os_timer_disarm(&timerInterrupt);
    }
  }
}

void TimerCallback(void *pArg) {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  indoorLight = analogRead(LDR_Pin);
  
  if(temperature != lastTemp)
  {
    client.publish(TEMP_TOPIC, String(temperature).c_str());
    lastTemp = temperature;
  }
  if(humidity != lastHumi)
  {
    client.publish(HUMI_TOPIC, String(humidity).c_str());
    lastHumi = humidity;
  }
  if(indoorLight != lastLight)
  {
    client.publish(IN_LDR_TOPIC, String(indoorLight).c_str());
    lastLight = indoorLight;
  }
}

void setup() {
  Serial.begin(115200);
  InitMQTTProtocol();
  dht.begin();
  os_timer_setfn(&timerInterrupt, TimerCallback, NULL);
}

void loop() {
  if (!client.connected()) {
    ConnectToBroker();
  }
  client.loop();
}