#include "HixConfig.h"
#include "secret.h"
#include <EEPROM.h>

HixConfig::HixConfig() {
    EEPROM.begin(512);
    EEPROM.get(m_nEPPROMBaseAddress, data);
    if (calculateCRC() != data.crc) {
        commitDefaults();
    }
}

const char * HixConfig::getUDPServer(void) {
    return data.szUDPServer;
};

const char * HixConfig::getRoom(void) {
    //   return "test_room";
    return data.szRoom;
};

const char * HixConfig::getDeviceTag(void) {
    //  return "test_tag";
    return data.szDeviceTag;
};

bool HixConfig::getOTAEnabled(void) {
    return data.bOTAEnabled;
}

void HixConfig::setUDPServer(const char * szValue) {
    memset(data.szUDPServer, 0, sizeof(data.szUDPServer));
    strncpy(data.szUDPServer, szValue, sizeof(data.szUDPServer) - 1);
}

void HixConfig::setRoom(const char * szValue) {
    memset(data.szRoom, 0, sizeof(data.szRoom));
    strncpy(data.szRoom, szValue, sizeof(data.szRoom) - 1);
}

void HixConfig::setDeviceTag(const char * szValue) {
    memset(data.szDeviceTag, 0, sizeof(data.szDeviceTag));
    strncpy(data.szDeviceTag, szValue, sizeof(data.szDeviceTag) - 1);
}

void HixConfig::setOTAEnabled(bool bValue) {
    data.bOTAEnabled = bValue;
}

void HixConfig::commitToEEPROM(void) {
    data.crc = calculateCRC();
    EEPROM.put(m_nEPPROMBaseAddress, data);
    EEPROM.commit();
}

unsigned long HixConfig::calculateCRC(void) {
    unsigned char * crcData = (unsigned char *)&data;
    size_t          size    = sizeof(data) - sizeof(data.crc);
    unsigned long   crc     = 1234;
    for (size_t index = 0; index < size; index++) {
        crc += crcData[index];
    }
    return crc;
}

void HixConfig::commitDefaults(void) {
    setMyIPAddress(Secret::MY_IP);
    setMySubnetMask(Secret::MY_MASK);
    setMyGateway(Secret::MY_GATEWAY);
    setOTAEnabled(true);
    setRoom("test_room");
    setDeviceTag("test_tag");
    setUDPServer(Secret::UDP_SERVER);
    setUDPPort(Secret::UDP_PORT);
    commitToEEPROM();
}

void HixConfig::replacePlaceholders(String & contents) {
    contents.replace("||WIFI_SSID||", getWifiSsid());
    contents.replace("||WIFI_PWD||", getWifiPassword());
    contents.replace("||MY_IP||", getMyIPAddress());
    contents.replace("||MY_MASK||", getMySubnetMask());
    contents.replace("||MY_GATEWAY||", getMyGateway());
    contents.replace("||OTA_ENABLED||", getOTAEnabled()? "checked":"");    
    contents.replace("||MY_ROOM||", getRoom());
    contents.replace("||MY_DEVICE_TAG||", getDeviceTag());
    contents.replace("||UDP_SERVER||", getUDPServer());
    contents.replace("||UDP_PORT||", String(getUDPPort()));
    contents.replace("||CONFIG_PWD||", getConfigPassword());
}