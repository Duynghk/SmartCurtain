#ifndef CONFIG_H
#define CONFIG_H

// Operating modes for the curtain system
#define AUTO_MODE true  
#define MANUAL_MODE false
#define DEFAULT_MODE AUTO_MODE

// Default values
#define DEFAULT_HIGH_TEMP 28
#define DEFAULT_LOW_TEMP 23
#define DEFAULT_HIGH_HUMI 80
#define DEFAULT_LOW_HUMI 50
#define WINDOW_CLOSE_ANGLE 0
#define WINDOW_OPEN_ANGLE 130

//Use for EEPROM
#define MEMORY_NAMESPACE "NodeMemory"
#define MODE_KEY "mode"
#define HIGH_HUMI_THRESHOLD_KEY "highHumiThreshold"
#define LOW_HUMI_THRESHOLD_KEY "lowHumiThreshold"
#define HIGH_TEMP_THRESHOLD_KEY "highTempThreshold"
#define LOW_TEMP_THRESHOLD_KEY "lowTempThreshold"
// Curtain status
#define CLOSE 0
#define OPEN 1

// Device states
#define OFF 0
#define ON 1
#define DARK 1
#define LIGHT 0

// Wi-Fi network information
#define MY_SSID "Khánh Duy"
#define PASSWORD "0982104532"

// MQTT broker information
#define MQTT_SERVER "broker.emqx.io"
#define MQTT_PORT 1883
#define CLIENT_ID "CurtainSystemNode"

// MQTT topics
#define CTRL_TOPIC "Smarthome/ControlWindowSystem"
#define INDOOR_TEMP_TOPIC "Smarthome/TempIndoor"
#define OUTDOOR_TEMP_TOPIC "Smarthome/TempOutdoor"
#define INDOOR_HUMI_TOPIC "Smarthome/HumiIndoor"
#define OUTDOOR_HUMI_TOPIC "Smarthome/HumiIndoor"
#define INDOOR_LIGHT_TOPIC "Smarthome/LightIndoor"
#define OUTDOOR_LIGHT_TOPIC "Smarthome/LightOutdoor"
#define RAIN_TOPIC "Smarthome/Rain"
#define STATUS_TOPIC "SmartHome/UpdateStatusNodes"
#define ERROR_TOPIC "SmartHome/ErrorWarning"
#define ALARM_TOPIC "SmartHome/Alarm"

// Control messages
#define TURN_ON_NODE "NodeOn"
#define TURN_OFF_NODE "NodeOff"
#define SET_AUTO_MODE "AutoMode"
#define SET_MANUAL_MODE "ManualMode"
#define CLOSE_WINDOW "CloseWindow"
#define OPEN_WINDOW "OpenWindow"
#define SET_HIGH_TEMP_THRESHOLD "SH"
#define SET_LOW_TEMP_THRESHOLD "SL"
#define SET_HIGH_HUMI_THRESHOLD "HH"
#define SET_LOW_HUMI_THRESHOLD "HL"

// Curtain status messages
#define WINDOW_CLOSED "WindowClosed"
#define WINDOW_OPENED "WindowOpened"
#define MODE_CHANGED "ModeChanged"

// Light sensor messages
#define DARK_STRING "Dark"
#define LIGHT_STRING "Light"

// Rain Sensor
#define RAIN true
#define NO_RAIN false
#define RAIN_STRING "true"
#define NO_RAIN_STRING "false"

// Alarm smoke and gas
#define WARN_MESS "WARNING"

// ERROR 
#define AC_ERROR "ACError" //air condition
#define C_ERROR "CurtainError" //curtain
#endif
