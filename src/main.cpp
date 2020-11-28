#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <FS.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <HixMQTTBase.h>
#include <HixPinDigitalInput.h>
#include "HixWebServer.h"
#include "secret.h"
#include "HixConfig.h"
#include <Adafruit_NeoPixel.h>

ADC_MODE(ADC_VCC);

HixConfig g_config;
HixWebServer g_webServer(g_config);
HixPinDigitalInput g_pinStayAwake(4);
WiFiUDP g_udp;
Adafruit_NeoPixel g_rgbLed = Adafruit_NeoPixel(1, 5, NEO_GRB + NEO_KHZ400);

enum Color : uint32_t
{
    black = 0x000000,
    white = 0xFFFFFF,
    red = 0xFF0000,
    green = 0x00FF00,
    blue = 0x0000FF,
    orange = 0xFF8C00
};

void setLedColor(Color color, u32_t nDimFactor = 16)
{
    //make base colors
    u32_t nR = color >> 16;
    u32_t nG = (color & 0x00FF00) >> 8;
    u32_t nB = color & 0x0000FF;
    //take dimming into account
    nR /= nDimFactor;
    nG /= nDimFactor;
    nB /= nDimFactor;
    //reassemble
    u32_t nDimmedColor = (nR << 16) | (nG << 8) | nB;
    g_rgbLed.setPixelColor(0, nDimmedColor);
    g_rgbLed.show();
}

void resetWithMessage(const char *szMessage)
{
    Serial.println(szMessage);
    delay(2000);
    ESP.reset();
}

// this is accessed when the initial run fails to connect because no (or old) credentials
void launchSlowConnect()
{
    Serial.println("No (or wrong) saved WiFi credentials. Doing a fresh connect.");
    // persistent and autoconnect should be true by default, but lets make sure.
    if (!WiFi.getAutoConnect())
        WiFi.setAutoConnect(true); // autoconnect from saved credentials
    if (!WiFi.getPersistent())
        WiFi.persistent(true); // save the wifi credentials to flash

    // Note the two forms of WiFi.begin() below. If the first version is used
    // then no wifi-scan required as the RF channel and the AP mac-address are provided.
    // so next boot, all this info is saved for a fast connect.
    // If the second version is used, a scan is required and for me, adds about 2-seconds
    // to my connect time. (on a cold-boot, ie power-cycle)

    //WiFi.begin(ssid, password, channel, home_mac, true);
    WiFi.begin(Secret::WIFI_SSID, Secret::WIFI_PWD);

    // now wait for good connection, or reset
    int counter = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
        if (++counter > 20)
        { // allow up to 10-sec to connect to wifi
            resetWithMessage("WIFI timed-out, resetting...");
        }
    }

    Serial.println("WiFi connected and credentials saved");
}

float getVcc(void)
{
    const int numberOfSamples = 100;
    float sum = 0;
    for (int i = 0; i < numberOfSamples; i++)
    {
        sum += ((float)ESP.getVcc()) / 1024;
    }
    return sum / (float)numberOfSamples;
}

void sendUDPpacket()
{
    //create payload
    DynamicJsonDocument doc(500);
    doc["device_type"] = g_config.getDeviceType();
    doc["device_version"] = g_config.getDeviceVersion();
    doc["device_tag"] = g_config.getDeviceTag();
    doc["room"] = g_config.getRoom();
    doc["wifi_ssid"] = WiFi.SSID();
    doc["wifi_rssi"] = WiFi.RSSI();
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

void setLedForVcc()
{
    float vcc = getVcc();
    if (vcc < 2.60)
    {
        setLedColor(Color::red);
        return;
    }
    if (vcc < 2.8)
    {
        setLedColor(Color::orange);
        return;
    }
    setLedColor(Color::green);
}

// ===================================================================================
// note that there is no WiFi.begin() in the setup.
// It IS accessed in a function the 1st time the sketch is run
// so that the details can be stored, but any restarts after that
// are connected by the saved credentials and it happens MUCH faster.

void setup(void)
{
    Serial.begin(115200);
    g_pinStayAwake.begin();
    pinMode(g_pinStayAwake.getPinNumber(), INPUT_PULLUP);
    // a significant part of the speed gain is by using a static IP config
    //WiFi.begin();
    WiFi.config(g_config.getIPAddress(), g_config.getGateway(), g_config.getSubnetMask());
    // so even though no WiFi.connect() so far, check and see if we are connecting.
    // The 1st time sketch runs, this will time-out and THEN it accesses WiFi.connect().
    // After the first time (and a successful connect), next time it connects very fast
    int counter = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(5); // use small delays, NOT 500ms
        if (++counter > 1000)
            break; // 5 sec timeout
    }
    //if timed-out, connect the slow-way
    if (counter > 1000)
        launchSlowConnect();
    //some debugging info
    Serial.println();
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
    //setup the server
    Serial.println(F("Setting up RGB lede"));
    g_rgbLed.begin();
    setLedColor(Color::blue);
    //setup the server
    Serial.println(F("Setting up web server"));
    g_webServer.begin();
    //setup SPIFFS
    Serial.println(F("Setting up SPIFFS"));
    if (!SPIFFS.begin())
        resetWithMessage("SPIFFS initialization failed, resetting");
    //show led color
    setLedForVcc();
    //give some time for message sending in background
    delay(1250);
}

// ===================================================================================

// ------------------------------------------------------------------------------------------
void loop(void)
{
    if (g_pinStayAwake.isHigh())
    {
        g_webServer.handleClient();
        setLedForVcc();
    }
    else
    {
        //debug log
        Serial.println("Going to sleep...");
        //switch off let
        setLedColor(Color::black);
        //go into deepsleep!
        ESP.deepSleep(0);
    }
}