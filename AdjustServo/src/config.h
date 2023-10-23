#ifndef CONFIG_H
#define CONFIG_H

//Use for EEPROM
const char* MEMORY_NAMESPACE = "NodeMemory";
const char* CLOSE_ANGLE_KEY = "closeAngle";
const char* OPEN_ANGLE_KEY = "openAngle";

//System states
#define ON  1
#define OFF 0

#define DEFAULT_CLOSE_ANGLE  0
#define DEFAULT_OPEN_ANGLE 50

// Wi-Fi network information
#define SSID "Khánh Duy"
#define PASSWORD "0982104532"

// MQTT broker information
#define MQTT_SERVER "broker.emqx.io"
#define MQTT_PORT 1883
#define CLIENT_ID "ServoInSmartHome"

// MQTT topics
#define ADJUST_TOPIC "Smarthome/AdjustServo"

// Callback message
#define ON_MESSAGE "SystemOn"
#define OFF_MESSAGE "SystemOff"
#define SET_CLOSE_ANGLE "SC"
#define SET_OPEN_ANGLE "SO"
// LED connection pin
#define LED 2

#endif
