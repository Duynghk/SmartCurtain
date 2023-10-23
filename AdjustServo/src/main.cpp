#include<ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Preferences.h>
#include <Servo.h>
#include <config.h>

#define CLOSE_OPEN_INTERVAL 3000

WiFiClient espClient;
PubSubClient client(espClient);
os_timer_t timerInterrupt;
Servo servo;
Preferences memory;

int systemState = ON;
int closeAngle = DEFAULT_CLOSE_ANGLE;
int openAngle = DEFAULT_OPEN_ANGLE;


volatile int servoAngle = 0;

void ReadUserConfig();
void SetupWifi();
void ConnectToBroker();
void InitMQTTProtocol();
void MQTTCallback(char* , byte* , unsigned int);
void TimerCallback(void *);

void ReadUserConfig() 
{
  memory.begin(MEMORY_NAMESPACE, false);
  closeAngle = memory.getInt(CLOSE_ANGLE_KEY, DEFAULT_CLOSE_ANGLE);
  openAngle = memory.getInt(OPEN_ANGLE_KEY, DEFAULT_OPEN_ANGLE);
  Serial.println("Memory is read");
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
      client.subscribe(ADJUST_TOPIC);
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
  char identifier[3];
  strncpy(identifier, message, 2);
  identifier[2] = '\0';
  if(strcmp(identifier, SET_CLOSE_ANGLE) == 0) 
  {
    const char* data = message + 2;
    closeAngle = atoi(data);
    memory.putInt(CLOSE_ANGLE_KEY, closeAngle);
  }
  else if(strcmp(identifier, SET_OPEN_ANGLE) == 0) 
  {
    const char* data = message + 2;
    openAngle = atoi(data);
    memory.putInt(OPEN_ANGLE_KEY, openAngle);
  }
  else if(strcmp(message, ON_MESSAGE) == 0) 
  {
    os_timer_arm(&timerInterrupt, CLOSE_OPEN_INTERVAL, true);
    systemState = ON;
  }
  else if(strcmp(message, OFF_MESSAGE) == 0)
  {
    systemState = OFF;
    os_timer_disarm(&timerInterrupt);
  }
}

void TimerCallback(void *pArg) {
  // Điều khiển servo bằng góc servoAngle
  if (servoAngle == closeAngle) servoAngle = openAngle;
  else servoAngle = closeAngle;
  servo.write(servoAngle);
}

void setup() {
  Serial.begin(115200);
  ReadUserConfig();
  InitMQTTProtocol();
  pinMode(LED,OUTPUT);
  servo.attach(0);  // Use PIN 0 to cotrol servo
  os_timer_setfn(&timerInterrupt, TimerCallback, NULL);
  os_timer_arm(&timerInterrupt, CLOSE_OPEN_INTERVAL, true);
}

void loop() {
  if (!client.connected()) {
    ConnectToBroker();
  }
  client.loop();
  if (systemState == ON) 
  {
    Serial.print("Close angle: ");
    Serial.println(closeAngle);
    Serial.print("Open angle: ");
    Serial.println(openAngle);
    digitalWrite(LED,LOW);
    delay(1000);
    digitalWrite(LED,HIGH);
    delay(1000);
  }
}
