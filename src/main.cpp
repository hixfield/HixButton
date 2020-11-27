#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <FS.h>
#include <Arduino.h>
#include <ArduinoJson.h>

//#include "HixWebServer.h"
#include "secret.h"
#include "HixConfig.h"

ADC_MODE(ADC_VCC);

// ---------------------------------------------------------

// change to suit your local network
#define IPA IPAddress(192, 168, 99, 5)
#define GATE IPAddress(192, 168, 99, 1)
#define MASK IPAddress(255, 255, 255, 0)

// address of a UDP listener on the same network
char UDPtarget[] = {"192.168.99.4"}; // **
const uint16_t udpPort = 63001;      // ** UDP port number
const String UDPtestMessage = "Hi, this is a test message by UDP";

WiFiUDP udp;

// ------------------------------------------------------------------------------------------
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
            Serial.println("wifi timed-out. Rebooting..");
            delay(10); // so the serial message has time to get sent
            ESP.restart();
        }
    }

    Serial.println("WiFi connected and credentials saved");
}

void sendUDPpacket()
{
    Serial.print("sending UDP packet to ");
    Serial.print(UDPtarget);
    Serial.print(" on port ");
    Serial.print(udpPort);
    udp.beginPacket(UDPtarget, udpPort);
    udp.print(UDPtestMessage);
    udp.endPacket();
    Serial.print(" with contents: ");
    Serial.println(UDPtestMessage);
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

// ===================================================================================
// note that there is no WiFi.begin() in the setup.
// It IS accessed in a function the 1st time the sketch is run
// so that the details can be stored, but any restarts after that
// are connected by the saved credentials and it happens MUCH faster.

void setup(void)
{
    Serial.begin(115200);
    // a significant part of the speed gain is by using a static IP config
    WiFi.config(IPA, GATE, MASK);
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

    Serial.print("Vcc: ");
    Serial.println(getVcc());
}

// ===================================================================================

// ------------------------------------------------------------------------------------------
void loop(void)
{
    //send our packet
    sendUDPpacket();
    //give some time for message sending in background
    delay(1250);
    //debug log
    Serial.println("Going to sleep...");
    //go into deepsleep!
    ESP.deepSleep(0);
}