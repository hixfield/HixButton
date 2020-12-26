#include "secret.h"

//Contains the default settings used when the stored configuration is invalid.
//Like when the device is first booted.

const char *Secret::CONFIG_PWD = "bornem1973";

const char *Secret::WIFI_SSID = "GEMIC_2.4Ghz";
const char *Secret::WIFI_PWD = "ingegeertellen";
const bool Secret::FIXED_IP_ENABLED = true;
const char *Secret::MY_IP = "192.168.99.5";
const char *Secret::MY_MASK = "255.255.255.0";
const char *Secret::MY_GATEWAY = "192.168.99.1";

const bool Secret::OTA_ENABLED = false;  //not implemented (yet)
const char *Secret::MY_ROOM = "test_room";
const char *Secret::MY_DEVICE_TAG = "test_tag";
const char *Secret::UDP_SERVER = "192.168.99.4";
const int Secret::UDP_PORT = 63001;