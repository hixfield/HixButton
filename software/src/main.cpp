#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <HixMQTTBase.h>
#include <HixPinDigitalInput.h>
#include <HixTimeout.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

#include "HixConfig.h"
#include "HixRGBLed.h"
#include "HixWebServer.h"
#include "secret.h"

ADC_MODE(ADC_VCC);

HixConfig g_config;
HixWebServer g_webServer(g_config);
HixPinDigitalInput g_pinStayAwake(4);
WiFiUDP g_udp;
char g_szUdpPacketBuffer[512];
bool g_bUdpPacketReceived = false;
HixRGBLed g_rgbLed(5, NEO_GRB + NEO_KHZ400, 25);
HixTimeout g_accessPointTimeout(10 * 60 * 1000);
HixTimeout g_sleepTimeout(3000);

void resetWithMessage(const char *szMessage) {
    Serial.println(szMessage);
    delay(2000);
    ESP.reset();
}

// this is accessed when the initial run fails to connect because no (or old) credentials
bool launchSlowConnect() {
    Serial.println("No (or wrong) saved WiFi credentials. Doing a fresh connect.");
    // persistent and autoconnect should be true by default, but lets make sure.
    if (!WiFi.getAutoConnect())
        WiFi.setAutoConnect(true);  // autoconnect from saved credentials
    if (!WiFi.getPersistent())
        WiFi.persistent(true);  // save the wifi credentials to flash
    // Note the two forms of WiFi.begin() below. If the first version is used
    // then no wifi-scan required as the RF channel and the AP mac-address are provided.
    // so next boot, all this info is saved for a fast connect.
    // If the second version is used, a scan is required and for me, adds about 2-seconds
    // to my connect time. (on a cold-boot, ie power-cycle)
    WiFi.begin(g_config.getWifiSsid(), g_config.getWifiPassword());
    // now wait for good connection, or reset
    int counter = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        if (++counter > 20) {
            // allow up to 10-sec to connect to wifi
            Serial.println();
            Serial.println("Error connecting WiFi");
            return false;
        }
    }
    Serial.println();
    Serial.println("WiFi connected and credentials saved");
    return true;
}

float getVcc(void) {
    const int numberOfSamples = 100;
    float sum = 0;
    for (int i = 0; i < numberOfSamples; i++) {
        sum += (float)ESP.getVcc() / (float)1024;
    }
    return sum / (float)numberOfSamples;
}

void sendUDPpacket() {
    //create payload
    DynamicJsonDocument doc(500);
    doc["device_type"] = g_config.getDeviceType();
    doc["device_version"] = g_config.getDeviceVersion();
    doc["device_tag"] = g_config.getDeviceTag();
    doc["device_build_timestamp"] = g_config.getDeviceBuildTimestamp();
    doc["room"] = g_config.getRoom();
    doc["wifi_ssid"] = WiFi.SSID();
    doc["wifi_rssi"] = WiFi.RSSI();
    doc["wifi_mac"] = WiFi.macAddress();
    doc["vcc"] = getVcc();
    //to string
    String jsonString;
    serializeJson(doc, jsonString);
    //start sending...
    Serial.print("sending UDP packet to ");
    Serial.print(g_config.getUDPServerAsString());
    Serial.print(" on port ");
    Serial.print(g_config.getUDPPort());
    g_udp.beginPacket(g_config.getUDPServerAsString(), g_config.getUDPPort());
    g_udp.print(jsonString);
    g_udp.endPacket();
    Serial.print(" with contents: ");
    Serial.println(jsonString);
}

void setLedForVcc() {
    float vcc = getVcc();
    if (vcc < 2.80) {
        g_rgbLed.setColor(HixColor::red);
        g_rgbLed.setAnimate(HixLedAnimation::fastBlink);
    }
}

bool setupAccessPoint(void) {
    //create ssid name
    String ssid("HixButton_");
    ssid += g_config.getRoom();
    ssid += "_";
    ssid += g_config.getDeviceTag();
    //debug
    Serial.print("Setup access point with SSID: ");
    Serial.println(ssid);
    return WiFi.softAP(ssid);
}

bool accessPointIsActive(void) {
    return WiFi.softAPSSID() != NULL;
}

void setup(void) {
    Serial.begin(115200);
    Serial.println();
    Serial.println("Setting up stay awake pin");
    g_pinStayAwake.begin(INPUT_PULLUP);
    //setup the RGB
    Serial.println(F("Setting up RGB lede"));
    g_rgbLed.begin();
    Serial.println("Setting RGB led for Vcc level");
    //we set color here, for some strange reasen calling ESP.getVcc disconnects the wifi if we do it after wifi connection...
    setLedForVcc();
    //disable the accesspoint if it runs
    if (accessPointIsActive()) {
        Serial.println("Disabling soft access point");
        WiFi.softAPdisconnect();
    }
    //go for wifi connection
    Serial.println("Connecting WiFi");
    //check dhcp or not
    if (g_config.getFixedIPEnabled()) {
        // a significant part of the SPEED GAIN is by using a static IP config
        WiFi.config(g_config.getIPAddress(), g_config.getGateway(), g_config.getSubnetMask());
    } else {
        //this is using dhcp
        WiFi.begin();
    }
    // so even though no WiFi.connect() so far, check and see if we are connecting.
    // The 1st time sketch runs, this will time-out and THEN it accesses WiFi.connect().
    // After the first time (and a successful connect), next time it connects very fast
    int counter = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(5);  // use small delays, NOT 500ms
        if (++counter > 1000)
            break;  // 5 sec timeout
    }
    //if timed-out, connect the slow-way
    if (counter > 1000) {
        if (!launchSlowConnect()) {
            if (!setupAccessPoint()) {
                resetWithMessage("Could not setup a software access point");
            }
        }
    }
    //setup our UDP service
    Serial.print(F("Setting up UDP listener on port "));
    Serial.println(g_config.getUDPPort());
    g_udp.begin(g_config.getUDPPort());
    //debug logging
    if (accessPointIsActive()) {
        Serial.print("My access point IP: ");
        Serial.println(WiFi.softAPIP());
        g_rgbLed.setColor(HixColor::white);
        g_rgbLed.setAnimate(HixLedAnimation::fadeInOut);
    } else {
        //some debugging info
        Serial.print("Connected to: ");
        Serial.println(WiFi.SSID());
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        Serial.print("Subnetmask: ");
        Serial.println(WiFi.subnetMask());
        Serial.print("Gateway: ");
        Serial.println(WiFi.gatewayIP());
        Serial.print("Vcc: ");
        Serial.println(getVcc());
        //send our packet
        sendUDPpacket();
    }
    //setup mdns responder
    Serial.print(F("Setting up mDNS on "));
    Serial.println(g_config.getClientName());
    if (!MDNS.begin(g_config.getClientName())) {
        Serial.print(F("Error setting up mDNS responder"));
    }
    //setup the server
    Serial.println(F("Setting up web server"));
    g_webServer.begin();
    //setup SPIFFS
    Serial.println(F("Setting up SPIFFS"));
    if (!SPIFFS.begin())
        resetWithMessage("SPIFFS initialization failed, resetting");
    //start our sleep timeout
    g_sleepTimeout.restart();
    //all done!
    Serial.println(F("Setup complete, looping..."));
}

bool shouldGoToSleep(void) {
    //if running in access point
    if (accessPointIsActive()) {
        //if still people connected we don't disconnect
        if (WiFi.softAPgetStationNum()) {
            //Serial.println("Somebody connected");
            return false;
        }
        //if no body is connected check timeout
        return g_accessPointTimeout.isExpired();
    }
    //not running in AP mode then its the setting
    if (g_pinStayAwake.isLow()) return false;
    //no API, no pin stay awake the use timeout timer
    return g_sleepTimeout.isExpired();
}

void loop(void) {
    //handle any web sessions
    g_webServer.handleClient();
    //handle the mDNS
    MDNS.update();
    //should we sleep?
    if (shouldGoToSleep()) {
        //debug log
        Serial.println("Going to sleep...");
        //switch off let
        g_rgbLed.setColor(HixColor::black);
        g_rgbLed.setAnimate(HixLedAnimation::off);
        //go into deepsleep!
        ESP.deepSleep(0);
    }
    //if we are staying awake tell it via the led!
    if (g_pinStayAwake.isLow() && g_sleepTimeout.isExpired() && !g_bUdpPacketReceived) {
        g_rgbLed.setColor(HixColor::magenta);
        g_rgbLed.setAnimate(HixLedAnimation::fadeInOut);
    }
    //did we receive an udp packet?
    if (g_udp.parsePacket()) {
        g_bUdpPacketReceived = true;
        //make sure our buffer is zero terminated
        memset(g_szUdpPacketBuffer, 0, sizeof(g_szUdpPacketBuffer));
        //read in the packet
        g_udp.read(g_szUdpPacketBuffer, sizeof(g_szUdpPacketBuffer) - 1);
        //print debug stuff
        Serial.println("Received UDP packet:");
        Serial.println(g_szUdpPacketBuffer);
        //parse packet
        DynamicJsonDocument doc(512);
        deserializeJson(doc, g_szUdpPacketBuffer);
        const char *szLedColorRGBHex = doc["ledColorRGBHex"];
        unsigned long ulSleepDelayMs = doc["sleepDelayMs"];
        //check values
        if (szLedColorRGBHex != NULL) {
            Serial.println(szLedColorRGBHex);
            g_rgbLed.setColor(szLedColorRGBHex);
            if (g_pinStayAwake.isLow()) {
                g_rgbLed.setAnimate(HixLedAnimation::fadeInOut);
            } else {
                g_rgbLed.setAnimate(HixLedAnimation::on);
            }
            //wait a few seconds longer...
            g_sleepTimeout.updateTimeoutAndRestart(ulSleepDelayMs);
        }
    }
}