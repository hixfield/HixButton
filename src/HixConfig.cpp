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

const char * HixConfig::getWifiSsid(void) {
    return data.szWifiSsid;
}

const char * HixConfig::getWifiPassword(void) {
    return data.szWifiPassword;
}

IPAddress HixConfig::getIPAddress(void) {
    IPAddress address;
    address.fromString(getIPAddressAsString());
    return address;
}

const char * HixConfig::getIPAddressAsString(void) {
    return data.szIPAddress;
}

IPAddress HixConfig::getSubnetMask(void) {
    IPAddress address;
    address.fromString(getSubnetMaskAsString());
    return address;
}

const char * HixConfig::getSubnetMaskAsString(void) {
    return data.szSubnetMask;
}

IPAddress HixConfig::getGateway(void) {
    IPAddress address;
    address.fromString(getGatewayAsString());
    return address;
}

const char * HixConfig::getGatewayAsString(void) {
    return data.szGateway;
}

bool HixConfig::getOTAEnabled(void) {
    return data.bOTAEnabled;
}

const char * HixConfig::getConfigPassword(void) {
    return data.szConfigPassword;
}

const char * HixConfig::getRoom(void) {
    return data.szRoom;
}

const char * HixConfig::getDeviceTag(void) {
    return data.szDeviceTag;
}

const char * HixConfig::getUDPServerAsString(void) {
    return data.szUDPServer;
}

int HixConfig::getUDPPort(void) {
    return data.nUDPPort;
}

void HixConfig::setWifiSsid(const char * szValue) {
    memset(data.szWifiSsid, 0, sizeof(data.szWifiSsid));
    strncpy(data.szWifiSsid, szValue, sizeof(data.szWifiSsid) - 1);
}

void HixConfig::setWifiPassword(const char * szValue) {
    memset(data.szWifiPassword, 0, sizeof(data.szWifiPassword));
    strncpy(data.szWifiPassword, szValue, sizeof(data.szWifiPassword) - 1);
}

void HixConfig::setIPAddress(const char * szValue) {
    memset(data.szIPAddress, 0, sizeof(data.szIPAddress));
    strncpy(data.szIPAddress, szValue, sizeof(data.szIPAddress) - 1);
}

void HixConfig::setSubnetMask(const char * szValue) {
    memset(data.szSubnetMask, 0, sizeof(data.szSubnetMask));
    strncpy(data.szSubnetMask, szValue, sizeof(data.szSubnetMask) - 1);
}

void HixConfig::setGateway(const char * szValue) {
    memset(data.szGateway, 0, sizeof(data.szGateway));
    strncpy(data.szGateway, szValue, sizeof(data.szGateway) - 1);
}

void HixConfig::setOTAEnabled(bool bValue) {
    data.bOTAEnabled = bValue;
}

void HixConfig::setConfigPassword(const char * szValue) {
    memset(data.szConfigPassword, 0, sizeof(data.szConfigPassword));
    strncpy(data.szConfigPassword, szValue, sizeof(data.szConfigPassword) - 1);
}

void HixConfig::setRoom(const char * szValue) {
    memset(data.szRoom, 0, sizeof(data.szRoom));
    strncpy(data.szRoom, szValue, sizeof(data.szRoom) - 1);
}

void HixConfig::setDeviceTag(const char * szValue) {
    memset(data.szDeviceTag, 0, sizeof(data.szDeviceTag));
    strncpy(data.szDeviceTag, szValue, sizeof(data.szDeviceTag) - 1);
}

void HixConfig::setUDPServer(const char * szValue) {
    memset(data.szUDPServer, 0, sizeof(data.szUDPServer));
    strncpy(data.szUDPServer, szValue, sizeof(data.szUDPServer) - 1);
}

void HixConfig::setUDPPort(int nValue) {
        data.nUDPPort = nValue;
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
    setIPAddress(Secret::MY_IP);
    setSubnetMask(Secret::MY_MASK);
    setGateway(Secret::MY_GATEWAY);
    setOTAEnabled(true);
    setRoom("test_room");
    setDeviceTag("test_tag");
    setUDPServer(Secret::UDP_SERVER);
    setUDPPort(Secret::UDP_PORT);
    commitToEEPROM();
}

void HixConfig::replacePlaceholders(String & contents) {
    contents.replace("||DEVICE_TYPE||", getDeviceType());
    contents.replace("||DEVICE_VERSION||", getDeviceVersion());
    contents.replace("||WIFI_SSID||", getWifiSsid());
    contents.replace("||WIFI_PWD||", getWifiPassword());
    contents.replace("||MY_IP||", getIPAddressAsString());
    contents.replace("||MY_MASK||", getSubnetMaskAsString());
    contents.replace("||MY_GATEWAY||", getGatewayAsString());
    contents.replace("||OTA_ENABLED||", getOTAEnabled()? "checked":"");    
    contents.replace("||MY_ROOM||", getRoom());
    contents.replace("||MY_DEVICE_TAG||", getDeviceTag());
    contents.replace("||UDP_SERVER||", getUDPServerAsString());
    contents.replace("||UDP_PORT||", String(getUDPPort()));
    contents.replace("||CONFIG_PWD||", getConfigPassword());
}