#ifndef HixConfig_h
#define HixConfig_h

#include <arduino.h>

class HixConfig
{
private:
  //base address for epprom
  const int m_nEPPROMBaseAddress = 0;
  //hilds the data
  struct
  {
    //overall settings
    char szWifiSsid[50];
    char szWifiPassword[50];
    char szMyIPAddress[50];
    char szMySubnetMask[50];
    char szMyGateway[50];
    bool bOTAEnabled;
    char szConfigPassword[50];
    //my location stuff
    char szRoom[50];
    char szDeviceTag[50];
    //to where data is send
    char szUDPServer[50];
    int nUDPPort;
    //calculated CRC
    unsigned long crc;
  } data;
  //determine crc
  unsigned long calculateCRC(void);
  //store defaults
  void commitDefaults(void);

public:
  HixConfig();
  //getters
  const char *getDeviceType(void) { return "HixButton"; };
  const char *getDeviceVersion(void) { return "1.0.0"; };

  const char *getWifiSsid(void);
  const char *getWifiPassword(void);
  const char *getMyIPAddress(void);
  const char *getMySubnetMask(void);
  const char *getMyGateway(void);
  bool getOTAEnabled(void);
  const char *getConfigPassword(void);
  
  const char *getRoom(void);
  const char *getDeviceTag(void);

  const char *getUDPServer(void);
  int getUDPPort(void);

  //setters
  void setWifiSsid(const char *szValue);
  void setWifiPassword(const char *szValue);
  void setMyIPAddress(const char *szValue);
  void setMySubnetMask(const char *szValue);
  void setMyGateway(const char *szValue);
  void setOTAEnabled(bool bValue);

  void setRoom(const char *szValue);
  void setDeviceTag(const char *szValue);

  void setUDPServer(const char *szValue);
  void setUDPPort(int szValue);

  //save to eeprom
  void commitToEEPROM(void);
  //replaces placeholders in string with config values
  void replacePlaceholders(String &contents);
};

#endif
