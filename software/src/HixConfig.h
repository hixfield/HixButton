#ifndef HixConfig_h
#define HixConfig_h

#include <arduino.h>
#include <ipaddress.h>

class HixConfig
{
private:
  //base address for epprom
  const int m_nEPPROMBaseAddress = 0;
  char m_szClientName[255];
  //hilds the data
  struct
  {
    //overall settings
    char szConfigPassword[50];
    char szWifiSsid[50];
    char szWifiPassword[50];
    bool bFixedIPEnabled;
    char szIPAddress[50];
    char szSubnetMask[50];
    char szGateway[50];
    bool bOTAEnabled;
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
  const char *getDeviceVersion(void) { return "1.1.2"; };

  const char *getConfigPassword(void);
  const char *getWifiSsid(void);
  const char *getWifiPassword(void);
  bool getFixedIPEnabled(void);
  const char *getIPAddressAsString(void);
  IPAddress getIPAddress(void);
  const char *getSubnetMaskAsString(void);
  IPAddress getSubnetMask(void);
  const char *getGatewayAsString(void);
  IPAddress getGateway(void);
  bool getOTAEnabled(void);

  const char *getRoom(void);
  const char *getDeviceTag(void);

  const char *getUDPServerAsString(void);
  int getUDPPort(void);

  char * getClientName(void);

  //setters
  void setConfigPassword(const char *szValue);
  void setWifiSsid(const char *szValue);
  void setWifiPassword(const char *szValue);
  void setFixedIPEnabled(bool bValue);
  void setIPAddress(const char *szValue);
  void setSubnetMask(const char *szValue);
  void setGateway(const char *szValue);
  void setOTAEnabled(bool bValue);

  void setRoom(const char *szValue);
  void setDeviceTag(const char *szValue);

  void setUDPServer(const char *szValue);
  void setUDPPort(int nValue);

  //save to eeprom
  void commitToEEPROM(void);
  //replaces placeholders in string with config values
  void replacePlaceholders(String &contents);
};

#endif
