#include<Arduino.h>
#include "config.h"
#include <Preferences.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define MAX_WAIT_TIME 60000
#define SENSOR_READING_INTERVAL 1000
#define DEFAULT_HIGH_TEMP 28
#define DEFAULT_LOW_TEMP 23
#define LIGHT_OUTDOOR_PIN 0


WiFiClient espClient;
PubSubClient client(espClient);
Preferences memory;
os_timer_t sensorTriggeredHalt;

unsigned long startTime = 0;
int stateNode = OFF;
bool nodeMode = DEFAULT_MODE;
int curtainStatus = CLOSE;
int lastCurtainStatus = CLOSE;
int control = CLOSE;
int controlMessage = OFF;
float temperature = 0;
int indoorLight = 0;
int outdoorLight = 0;
int highTempThreshold = DEFAULT_HIGH_TEMP;
int lowTempThreshold = DEFAULT_LOW_TEMP;
bool tempValid = false;
bool lightValid = false;


void ReadUserConfig();
void SetupWifi();
void ConnectToBroker();
void InitMQTTProtocol();
void MQTTCallback(char* , byte* , unsigned int);
void SetMode(char *);
void ReadLightSensor(void *);

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
  if (strcmp(topic, CTRL_TOPIC) == 0) 
  {
    if(strcmp(message, "NodeOn") == 0)
    {
      stateNode = ON;
      startTime = millis();
      os_timer_arm(&sensorTriggeredHalt, SENSOR_READING_INTERVAL, true);
      //todo: call read senor funtion
    } 
    else if(strcmp(message, "NodeOff") == 0)
    {
      stateNode = OFF;
      curtainStatus = CLOSE;
      lastCurtainStatus = CLOSE;
      os_timer_disarm(&sensorTriggeredHalt);
      //control servo
      client.publish(STATUS_TOPIC, "CurtainClose");
    }
    else if(strcmp(message, "AutoMode") == 0)
    {
      nodeMode = AUTO_MODE;
      memory.putBool(MODE_KEY, AUTO_MODE);
      Serial.println("The auto mode is set");
    }
    else if(strcmp(message, "ManualMode") == 0)
    {
      nodeMode = MANUAL_MODE;
      memory.putBool(MODE_KEY, MANUAL_MODE);
      Serial.println("The manual mode is set");
    }
    else if(strcmp(message, "CloseCurtain") == 0)
    {
      curtainStatus = CLOSE;
    }
    else if(strcmp(message, "OpenCurtain") == 0)
    {
      curtainStatus = OPEN;
    } 
    else
    {
      char identifier[3];
      strncpy(identifier, message, 2);
      identifier[2] = '\0';
      const char* data = message + 2;
      if(strcmp(identifier, "SH") == 0)
      {
        highTempThreshold = atoi(data);
        memory.putInt(HIGH_TEMP_THRESHOLD_KEY, highTempThreshold);
      } else
        if(strcmp(identifier, "SL") == 0)
        {
          lowTempThreshold = atoi(data);
          memory.putInt(LOW_TEMP_THRESHOLD_KEY, lowTempThreshold);
        }
    }   
  } 
  else 
  {
    if (strcmp(topic, TEMP_TOPIC) == 0) {
      temperature = atof(message);
      tempValid = true;
    } else
      if (strcmp(topic, IN_LDR_TOPIC) == 0) {
        indoorLight = atoi(message);
        lightValid = true;
      } 
  }
}

void ReadLightSensor(void *pArg) {
  Serial.print("Timer interrupt count: ");
}

void setup() {
  Serial.begin(115200);
  ReadUserConfig();
  InitMQTTProtocol();
  pinMode(LED,OUTPUT);
  os_timer_setfn(&sensorTriggeredHalt, ReadLightSensor, NULL);
  os_timer_arm(&sensorTriggeredHalt, SENSOR_READING_INTERVAL, true);
}
void loop() {
  if (!client.connected()) {
    ConnectToBroker();
  }
  client.loop();
  checkSTATE:if(stateNode) 
  {
    if(nodeMode == AUTO_MODE)
    {
      if(tempValid == true && lightValid == true)
      {
        if(temperature < highTempThreshold)
        {
          if(temperature < lowTempThreshold)
          {
            if(outdoorLight == DARK) {curtainStatus = CLOSE;}
            else {curtainStatus = OPEN;}
          } 
          else
          {
            if(indoorLight == DARK && outdoorLight == LIGHT) {curtainStatus = OPEN;}
            else {curtainStatus = CLOSE;}
          }
        }
        else {curtainStatus = CLOSE;}
      }
      else
      {
        if(millis() - startTime > MAX_WAIT_TIME)
        {
          nodeMode = MANUAL_MODE;
          client.publish(CTRL_TOPIC, "ModeChanged");
          client.publish(ERROR_TOPIC, "ACError");
        }
        goto checkSTATE;
      }
    }
    if(lastCurtainStatus != curtainStatus)
    {
      lastCurtainStatus = curtainStatus;
      //to do: ctrl servo
      if(curtainStatus == CLOSE)
      {
        client.publish(STATUS_TOPIC, "CurtainCose");
      }
      else
      {
        client.publish(STATUS_TOPIC, "CurtainOpen");
      }
    }
  }
}
