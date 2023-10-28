#ifndef CONFIG_H
#define CONFIG_H

// Operating modes for the curtain system
#define AUTO_MODE true  
#define MANUAL_MODE false
#define DEFAULT_MODE AUTO_MODE

// Default values
#define DEFAULT_HIGH_TEMP 28
#define DEFAULT_LOW_TEMP 23
#define CURTAIN_CLOSE_ANGLE 0
#define CURTAIN_OPEN_ANGLE 130

//Use for EEPROM
#define MEMORY_NAMESPACE "NodeMemory"
#define MODE_KEY "mode"
#define HIGH_TEMP_THRESHOLD_KEY "highTempThreshold"
#define LOW_TEMP_THRESHOLD_KEY "lowTempThreshold"

// Curtain status
#define CLOSE 0
#define OPEN 1

// Device states (ON/OFF or LIGHT/DARK)
#define OFF 0
#define ON 1
#define DARK 0
#define LIGHT 1

// Wi-Fi network information
#define MY_SSID "Khánh Duy"
#define PASSWORD "0982104532"

// MQTT broker information
#define MQTT_SERVER "broker.emqx.io"
#define MQTT_PORT 1883
#define CLIENT_ID "CurtainSystemNode"

// MQTT topics
#define CTRL_TOPIC "Smarthome/ControlCurtainSystem"
#define INDOOR_TEMP_TOPIC "Smarthome/Temperature"
#define INDOOR_LIGHT_TOPIC "Smarthome/LightIndoor"
#define OUTDOOR_LIGHT_TOPIC "Smarthome/LightOutdoor"
#define STATUS_TOPIC "SmartHome/UpdateStatusNodes"
#define ERROR_TOPIC "SmartHome/ErrorWarning"

// Control messages
#define TURN_ON_NODE "NodeOn"
#define TURN_OFF_NODE "NodeOff"
#define SET_AUTO_MODE "AutoMode"
#define SET_MANUAL_MODE "ManualMode"
#define CLOSE_CURTAIN "CloseCurtain"
#define OPEN_CURTAIN "OpenCurtain"
#define SET_HIGH_TEMP_THRESHOLD "SH"
#define SET_LOW_TEMP_THRESHOLD "SL"

// Curtain status messages
#define CURTAIN_CLOSED "CurtainClosed"
#define CURTAIN_OPENED "CurtainOpened"
#define MODE_CHANGED "ModeChanged"

// Light sensor messages
#define DARK_STRING "DARK"
#define LIGHT_STRING "LIGHT"

// ERROR 
#define AC_ERROR "ACError"
#endif
