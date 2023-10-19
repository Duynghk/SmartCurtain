#include<Arduino.h>
#include "config.h"
#include <Preferences.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define SENSOR_READING_INTERVAL 1000
#define DEFAULT_HIGH_TEMP 28
#define DEFAULT_LOW_TEMP 23
#define LIGHT_OUTDOOR_PIN 0


WiFiClient espClient;
PubSubClient client(espClient);
Preferences memory;
os_timer_t timerInterrupt;

bool nodeMode = DEFAULT_MODE;
int curtainStatus = CLOSE;
int control = CLOSE;
int controlMessage = OFF;
float temperature = 0;
int indoorLight = 0;
int outdoorLight = 0;
int highTempThreshold = DEFAULT_HIGH_TEMP;
int lowTempThreshold = DEFAULT_LOW_TEMP;

void ReadUserConfig();
void SetupWifi();
void ConnectToBroker();
void InitMQTTProtocol();
void MQTTCallback(char* , byte* , unsigned int);
void SetMode(char *);
void TimerCallback(void *);

void ReadUserConfig() 
{
  memory.begin(MEMORY_NAMESPACE, false); 
  nodeMode = memory.getBool(MODE_KEY, DEFAULT_MODE);
  highTempThreshold = memory.getInt(HIGH_TEMP_THRESHOLD_KEY, DEFAULT_HIGH_TEMP);
  lowTempThreshold = memory.getInt(LOW_TEMP_THRESHOLD_KEY, DEFAULT_LOW_TEMP);
  Serial.println("User config is read");
}
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

void SetMode(char * message)
{
  if(strcmp(message, AUTO_MODE_STRING) == 0) 
  {
    nodeMode = AUTO_MODE;
    memory.putBool(MODE_KEY, AUTO_MODE);
    Serial.println("The auto mode is set");
  }
  else if (strcmp(message, MANUAL_MODE_STRING) == 0)
  {
    nodeMode = MANUAL_MODE;
    memory.putBool(MODE_KEY, MANUAL_MODE);
    Serial.println("The manual mode is set");
  } 
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

void TimerCallback(void *pArg) {
  Serial.print("Timer interrupt count: ");
}

void setup() {
  Serial.begin(115200);
  ReadUserConfig();
  InitMQTTProtocol();
  pinMode(LED,OUTPUT);
  os_timer_setfn(&timerInterrupt, TimerCallback, NULL);
  os_timer_arm(&timerInterrupt, SENSOR_READING_INTERVAL, true);
}
void loop() {
  if (!client.connected()) {
    ConnectToBroker();
  }
  client.loop();
  if (nodeMode) 
  {
    digitalWrite(LED,HIGH);
    delay(500);
    digitalWrite(LED,LOW);
    delay(500);
  }
  else 
  {
    digitalWrite(LED,HIGH);
    delay(50);
    digitalWrite(LED,LOW);
    delay(50);
    digitalWrite(LED,HIGH);
    delay(50);
    digitalWrite(LED,LOW);
    delay(200);
  }
  delay(100);
}
