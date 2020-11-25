#include "HixMQTT.h"
#include <Arduino.h>
#include <ArduinoJson.h>

HixMQTT::HixMQTT(const char * szWifi_SSID,
                 const char * szWiFi_Password,
                 const char * szMQTT_Server,
                 const char * szDeviceType,
                 const char * szDeviceVersion,
                 const char * szRoom,
                 const char * szDeviceTag) : HixMQTTBase(szWifi_SSID, szWiFi_Password, szMQTT_Server, szDeviceType, szDeviceVersion, szRoom, szDeviceTag) {
}
