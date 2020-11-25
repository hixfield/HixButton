#ifndef HixMQTT_h
#define HixMQTT_h

#include <HixMQTTBase.h>


class HixMQTT : public HixMQTTBase {
  public:
    HixMQTT(const char * szWifi_SSID,
            const char * szWiFi_Password,
            const char * szMQTT_Server,
            const char * szDeviceType,
            const char * szDeviceVersion,
            const char * szRoom,
            const char * szDeviceTag);
};

#endif
