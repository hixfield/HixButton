#include "HixWebServer.h"
#include "secret.h"
#include <FS.h>

HixWebServer::HixWebServer(HixConfig & config) : ESP8266WebServer(80),
                                                 m_config(config) {
    onNotFound([this]() {
        if (!handleFileRead(uri()))
            send(404, "text/plain", "404: Not Found");
    });
}

String HixWebServer::getContentType(String filename) {
    if (filename.endsWith(".htm"))
        return "text/html";
    else if (filename.endsWith(".html"))
        return "text/html";
    else if (filename.endsWith(".css"))
        return "text/css";
    else if (filename.endsWith(".js"))
        return "application/javascript";
    else if (filename.endsWith(".png"))
        return "image/png";
    else if (filename.endsWith(".gif"))
        return "image/gif";
    else if (filename.endsWith(".jpg"))
        return "image/jpeg";
    else if (filename.endsWith(".ico"))
        return "image/x-icon";
    else if (filename.endsWith(".xml"))
        return "text/xml";
    else if (filename.endsWith(".pdf"))
        return "application/x-pdf";
    else if (filename.endsWith(".zip"))
        return "application/x-zip";
    else if (filename.endsWith(".gz"))
        return "application/x-gzip";
    return "text/plain";
}

bool HixWebServer::shouldReplacePlaceholders(String filename) {
    if (filename.endsWith(".htm")) return true;
    if (filename.endsWith(".html")) return true;
    return false;
}

// send the right file to the client (if it exists)
bool HixWebServer::handleFileRead(String path) {
    char buf[2500];
    Serial.println("handleFileRead: " + path);
    //check for postback of config data
    if (path.endsWith("/postconfig")) {
        handlePostConfig();
        return true;
    }
    // If a folder is requested, send the index file
    if (path.endsWith("/")) path += "index.html";
    // Get the MIME type
    String contentType = getContentType(path);
    if (SPIFFS.exists(path)) {
        File file = SPIFFS.open(path, "r");
        if (shouldReplacePlaceholders(path)) {
            //read file in memory and...
            size_t size = file.readBytes(buf, sizeof(buf));
            //...zero terminate
            buf[size] = 0;
            //create string of it
            String contents(buf);
            m_config.replacePlaceholders(contents);
            //send it
            send(200, contentType, contents);
        } else {
            streamFile(file, contentType);
        }
        //close our input file
        file.close();
        //return non error
        return true;
    }
    //file not found
    return false;
}


bool HixWebServer::handlePostConfig(void) {
    //check password
    if (arg("password") != m_config.getConfigPassword()) {
        send(400, "text/plain", "Bad password");
        return false;
    }
    //handle parameters
    //TODO add other variables
    /*
    if (hasArg("mqtt_server")) m_config.setMQTTServer(arg("mqtt_server").c_str());
    */
    if (hasArg("wifi_ssid")) m_config.setWifiSsid(arg("wifi_ssid").c_str());
    if (hasArg("wifi_pwd")) m_config.setWifiPassword(arg("wifi_pwd").c_str());
    m_config.setFixedIPEnabled(hasArg("fixed_ip_enabled"));
    if (hasArg("ip")) m_config.setIPAddress(arg("ip").c_str());
    if (hasArg("mask")) m_config.setSubnetMask(arg("mask").c_str());
    if (hasArg("gateway")) m_config.setGateway(arg("gateway").c_str());
    m_config.setOTAEnabled(hasArg("ota_enabled"));
    if (hasArg("room")) m_config.setRoom(arg("room").c_str());
    if (hasArg("device_tag")) m_config.setDeviceTag(arg("device_tag").c_str());
    if (hasArg("udp_server")) m_config.setUDPServer(arg("udp_server").c_str());
    if (hasArg("udp_port")) m_config.setUDPPort(arg("udp_port").toInt());
    //write to epprom
    m_config.commitToEEPROM();
    //send reply
    const char * szSavedHtml = "/saved.html";
    File file = SPIFFS.open(szSavedHtml, "r");
    streamFile(file, getContentType(String(szSavedHtml)));
    file.close();
    //errase wifi settings
    WiFi.disconnect();
    //reset!
    delay(1000);
    ESP.reset();
    return true;
}