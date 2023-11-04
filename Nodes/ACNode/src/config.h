#ifndef CONFIG_H
#define CONFIG_H

// Operating modes for the curtain system
#define AUTO_MODE true  
#define MANUAL_MODE false
#define DEFAULT_MODE AUTO_MODE

//Use for EEPROM
const char* MEMORY_NAMESPACE = "NodeMemory";
const char* MODE_KEY = "mode";
const char* HIGH_TEMP_THRESHOLD_KEY = "highTempThreshold";
const char* LOW_TEMP_THRESHOLD_KEY = "lowTempThreshold";


// Curtain status
#define CLOSE 0
#define OPEN 1

// Device states (ON/OFF or LIGHT/DARK)
#define OFF 0
#define ON 1
#define DARK 0
#define LIGHT 1

// Wi-Fi network information
#define SSID "Khánh Duy"
#define PASSWORD "0982104532"

// MQTT broker information
#define MQTT_SERVER "broker.emqx.io"
#define MQTT_PORT 1883
#define CLIENT_ID "CurtainSystemNode"

// MQTT topics
#define CTRL_TOPIC "Smarthome/ControlCurtainSystem"
#define TEMP_TOPIC "Smarthome/Temperature"
#define IN_LDR_TOPIC "Smarthome/LightIndoor"
#define OUT_LDR_TOPIC "Smarthome/LightOutdoor"

const char* AUTO_MODE_STRING = "auto";
const char* MANUAL_MODE_STRING = "manual";

// LED connection pin
#define LED 2

#endif
