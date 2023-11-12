#include<Arduino.h>
#include "config.h"
#include <Preferences.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>
#include "DHT.h"

#define MAX_WAIT_SENSOR_TIME 30000
#define SENSOR_READING_INTERVAL 1000
#define RAIN_SENSOR_PIN 3
#define SERVO_PIN 2
#define DHT_PIN 0
#define DHTTYPE DHT22

WiFiClient espClient;
PubSubClient client(espClient);
Preferences memory;
DHT dht(DHT_PIN, DHTTYPE);
os_timer_t sensorTriggeredHalt;
Servo servo;

 
unsigned long startTime = 0;
int stateNode = OFF;
bool nodeMode = DEFAULT_MODE;
int windowStatus = CLOSE;
int lastWindowStatus = CLOSE;
int indoorLight = 0;
int outdoorLight = 0;

float indoorTemp = 0;
float outdoorTemp = 0;
float lastOutDoorTemp = 0;
float indoorHumid = 0;
float outdoorHumid = 0;
float lastOutDoorHumid = 0;
bool rain = false;
int lastRain = false;
float highTempThreshold = DEFAULT_HIGH_TEMP;
float lowTempThreshold = DEFAULT_LOW_TEMP;
float highHumidThreshold = DEFAULT_HIGH_HUMID;
float lowHumidThreshold = DEFAULT_LOW_HUMID;
bool tempValid = false;
bool humidValid = false;
bool lightValid = false;
bool inlightValid = false;
bool outlightValid = false;
int alarmBell = OFF;

void ReadUserConfig();
void SetupWifi();
void ConnectToBroker();
void InitMQTTProtocol();
void MQTTCallback(char* , byte* , unsigned int);
void SetMode(char *);
void ReadDHTSensor(void *);

void ReadUserConfig() 
{
  memory.begin(MEMORY_NAMESPACE, false); 
  nodeMode = memory.getBool(MODE_KEY, DEFAULT_MODE);
  highTempThreshold = memory.getFloat(HIGH_TEMP_THRESHOLD_KEY, DEFAULT_HIGH_TEMP);
  lowTempThreshold = memory.getFloat(LOW_TEMP_THRESHOLD_KEY, DEFAULT_LOW_TEMP);
  highHumidThreshold = memory.getFloat(HIGH_HUMID_THRESHOLD_KEY, DEFAULT_HIGH_HUMID);
  lowHumidThreshold = memory.getFloat(LOW_HUMID_THRESHOLD_KEY, DEFAULT_LOW_HUMID);
  Serial.print("User config is read: ");
  Serial.print(nodeMode);
  Serial.print("\t");
  Serial.print(highTempThreshold);
  Serial.print("\t");
  Serial.print(lowTempThreshold);
  Serial.print("\n");
  Serial.print(highHumidThreshold);
  Serial.print("\t");
  Serial.print(lowHumidThreshold);
  Serial.print("\n");
}
void SetupWifi() {
  Serial.print("Connecting to ");
  Serial.println(MY_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(MY_SSID, PASSWORD);

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
      client.subscribe(INDOOR_TEMP_TOPIC);
      client.subscribe(INDOOR_LIGHT_TOPIC);
      client.subscribe(INDOOR_HUMID_TOPIC);
      client.subscribe(OUTDOOR_LIGHT_TOPIC);
      client.subscribe(ALARM_TOPIC);
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
  if(strcmp(message, SET_AUTO_MODE) == 0) 
  {
    nodeMode = AUTO_MODE;
    memory.putBool(MODE_KEY, AUTO_MODE);
    Serial.println("The auto mode is set");
  }
  else if (strcmp(message, SET_MANUAL_MODE) == 0)
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
  Serial.println(topic);
  Serial.println(message);
  if (strcmp(topic, CTRL_TOPIC) == 0) 
  {
    Serial.println("Control topic");
    if(strcmp(message, TURN_ON_NODE) == 0)
    {
      stateNode = ON;
      startTime = millis();
      os_timer_arm(&sensorTriggeredHalt, SENSOR_READING_INTERVAL, true);
      ReadDHTSensor(NULL);
      Serial.println("Received NodeOn");
    } 
    else if(strcmp(message, TURN_OFF_NODE) == 0)
    {
      stateNode = OFF;
      windowStatus = CLOSE;
      lastWindowStatus = CLOSE;
      os_timer_disarm(&sensorTriggeredHalt);
      servo.write(WINDOW_CLOSE_ANGLE);
      client.publish(STATUS_TOPIC, WINDOW_CLOSED);
      Serial.println("Smart Window is off");
    }
    else if(strcmp(message, SET_AUTO_MODE) == 0)
    {
      nodeMode = AUTO_MODE;
      memory.putBool(MODE_KEY, AUTO_MODE);
      Serial.println("The auto mode is set");
    }
    else if(strcmp(message, SET_MANUAL_MODE) == 0)
    {
      nodeMode = MANUAL_MODE;
      memory.putBool(MODE_KEY, MANUAL_MODE);
      Serial.println("The manual mode is set");
    }
    else if(strcmp(message, CLOSE_WINDOW) == 0)
    {
      windowStatus = CLOSE;
      Serial.println("Manual close");
    }
    else if(strcmp(message, OPEN_WINDOW) == 0)
    {
      windowStatus = OPEN;
      Serial.println("Manual open");
    } 
    else
    {
      Serial.println("Read High_Low TEMP");
      char identifier[3];
      strncpy(identifier, message, 2);
      identifier[2] = '\0';
      const char* data = message + 2;
      if(strcmp(identifier, SET_HIGH_TEMP_THRESHOLD) == 0)
      {
        highTempThreshold = atoi(data);
        memory.putInt(HIGH_TEMP_THRESHOLD_KEY, highTempThreshold);
      } 
      if(strcmp(identifier, SET_LOW_TEMP_THRESHOLD) == 0)
      {
        lowTempThreshold = atoi(data);
        memory.putInt(LOW_TEMP_THRESHOLD_KEY, lowTempThreshold);
      }
      if(strcmp(identifier, SET_HIGH_HUMID_THRESHOLD) == 0)
      {
        highHumidThreshold = atoi(data);
        memory.putInt(HIGH_HUMID_THRESHOLD_KEY, highHumidThreshold);
      }
      if(strcmp(identifier, SET_LOW_HUMID_THRESHOLD) == 0)
      {
        lowHumidThreshold = atoi(data);
        memory.putInt(LOW_HUMID_THRESHOLD_KEY, lowHumidThreshold);
      }
    }   
  } 
  else 
  {
    Serial.println("Received data from AC node");
    if (strcmp(topic, INDOOR_TEMP_TOPIC) == 0) 
    {
      indoorTemp = atof(message);
      tempValid = true;
      Serial.println("Received temperature indoor");
    } 
    else if (strcmp(topic, INDOOR_LIGHT_TOPIC) == 0) 
    {
      if(strcmp(message, DARK_STRING) == 0) 
      {
        indoorLight = DARK;
      }
      else 
      {
        indoorLight = LIGHT;
      }
      inlightValid = true;
      Serial.println("Received brightness indoor");
    } 
    else if(strcmp(topic, OUTDOOR_LIGHT_TOPIC) == 0)
    {
      if(strcmp(message, DARK_STRING) == 0) 
      {
        outdoorLight = DARK;
      }
      else 
      {
        outdoorLight = LIGHT;
      }
      outlightValid = true;
      Serial.println("Received brightness outdoor");
    }
    else if(strcmp(topic, ALARM_TOPIC) == 0)
    {
      if(strcmp(message, TOXIC_GAS_WARNING) == 0)
      {
        windowStatus = OPEN;
        alarmBell = ON;
      }
      else
      {
        alarmBell = OFF;
      }
      Serial.println("Received alarm message");
    }
  }
}

void ReadDHTSensor(void *pArg) {
  outdoorTemp = dht.readTemperature();
  outdoorHumid = dht.readHumidity();
  rain = digitalRead(RAIN_SENSOR_PIN);
  if(outdoorTemp != lastOutDoorTemp)
  {
    client.publish(OUTDOOR_TEMP_TOPIC, String(outdoorTemp).c_str());
    lastOutDoorTemp = outdoorTemp;
  }
  if(outdoorHumid != lastOutDoorHumid)
  {
    client.publish(OUTDOOR_HUMID_TOPIC, String(outdoorHumid).c_str());
    lastOutDoorHumid = outdoorHumid;
  }
  if(rain != lastRain) 
  {
    if(rain == RAIN) client.publish(RAIN_TOPIC, NO_RAIN_STRING);
    else client.publish(RAIN_TOPIC, RAIN_STRING);
    lastRain = rain;
  }
}

void setup() {
  Serial.begin(115200,SERIAL_8N1,SERIAL_TX_ONLY);
  ReadUserConfig();
  InitMQTTProtocol();
  dht.begin();
  pinMode(RAIN_SENSOR_PIN, INPUT_PULLUP);
  servo.attach(SERVO_PIN);
  servo.write(WINDOW_CLOSE_ANGLE);
  os_timer_setfn(&sensorTriggeredHalt, ReadDHTSensor, NULL);
  os_timer_arm(&sensorTriggeredHalt, SENSOR_READING_INTERVAL, true);
}
void loop() {
HEAD:
  if (!client.connected()) {
    ConnectToBroker();
  }
  client.loop();
  if(alarmBell == 1)
  {
    goto RUN_STATUS;
  }
  else if(stateNode == ON)
  {
    if(nodeMode == AUTO_MODE)
    {
      if(tempValid && humidValid && lightValid)
      {
        if(rain == true || outdoorLight == DARK || outdoorHumid == HIGH)
        {
          windowStatus = CLOSE;
        }
        else  if(indoorLight == LIGHT)
              {
                if(indoorTemp < lowTempThreshold)
                {
                  if(outdoorTemp < lowTempThreshold)
                  {
                    windowStatus = CLOSE;
                  }
                  else if(outdoorTemp > highTempThreshold)
                  {
                    if(outdoorHumid < lowHumidThreshold)
                    {
                      windowStatus = CLOSE;
                    }
                    else windowStatus = OPEN;
                  }
                  else windowStatus = OPEN;
                }
                else  if(indoorTemp > highTempThreshold)
                      {
                        if(outdoorTemp > highTempThreshold)
                        {
                          windowStatus = CLOSE;
                        }
                        else  if(outdoorTemp < lowTempThreshold)
                              {
                                if(outdoorHumid < lowHumidThreshold)
                                {
                                  windowStatus = CLOSE;
                                }
                                else windowStatus = OPEN;
                              }
                              else windowStatus = OPEN;
                        }
                        else  if(outdoorHumid < lowHumidThreshold) windowStatus = CLOSE;
                              else  if(outdoorTemp > highTempThreshold || outdoorTemp < lowTempThreshold)
                                    {
                                      if(lowHumidThreshold < indoorHumid && indoorHumid < highHumidThreshold)
                                      {
                                        windowStatus = CLOSE;
                                      }
                                      else windowStatus = OPEN;
                                    }
                                    else windowStatus = OPEN;
              }
              else  if(indoorTemp < lowTempThreshold)
                    {
                      if(lowTempThreshold < outdoorTemp && outdoorTemp < highTempThreshold)
                      {
                        windowStatus = OPEN;
                      }
                      else windowStatus = CLOSE;
                    }
                    else  if(indoorTemp > highTempThreshold)
                          {
                            if(lowTempThreshold < outdoorTemp && outdoorTemp < highTempThreshold)
                            {
                              windowStatus = OPEN;
                            }
                            else windowStatus = CLOSE;
                          }
                          else windowStatus = OPEN;
      }
      else
      {
        if(millis() - startTime > MAX_WAIT_SENSOR_TIME)
        {
          nodeMode = MANUAL_MODE;
          client.publish(CTRL_TOPIC, MODE_CHANGED);
          if(lightValid == false) client.publish(ERROR_TOPIC, AC_ERROR);
          if(tempValid == false || humidValid == false) client.publish(ERROR_TOPIC, CURTAIN_ERROR);
        }
        delay(100);
        goto HEAD;
      }  
    }
RUN_STATUS:
    if(lastWindowStatus != windowStatus)
    {
      lastWindowStatus = windowStatus;
      if(windowStatus == CLOSE)
      {
        servo.write(WINDOW_CLOSE_ANGLE);
        client.publish(STATUS_TOPIC, "WindowClose");
        Serial.println("Window Closed");
      }
      else
      {
        servo.write(WINDOW_OPEN_ANGLE);
        client.publish(STATUS_TOPIC, "WindowOpen");
        Serial.println("Window Opened");
      }
    }
    delay(1000);
  }
}
