#ifndef Secret_h
#define Secret_h

class Secret
{
public:
  static const char *WIFI_SSID;
  static const char *WIFI_PWD;
  static const char *MY_IP;
  static const char *MY_MASK;
  static const char *MY_GATEWAY;
  static const bool *OTA_ENABLED;
  static const char *CONFIG_PWD;

  static const char *MY_ROOM;
  static const char *MY_DEVICE_TAG;

  static const char *UDP_SERVER;
  static const int UDP_PORT;
};

#endif
