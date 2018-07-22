#ifndef SWITCH_H
#define SWITCH_H
 
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUDP.h>
#include "CallbackFunction.h"

 
class Switch {
private:
        ESP8266WebServer *server = NULL;
        WiFiUDP UDP;
        String serial;
        String persistent_uuid;
        String device_name;
        unsigned int localPort;
        CallbackFunction onCallback;
        CallbackFunction offCallback;
        CallbackFunction2 doorSensorCallback;
        void startWebServer();
        void handleEventservice();
        void handleUpnpControl();
        void handleOnOff();
        void doorStatus();
        void handleRoot();
        void handleSetupXml();
        
public:
        Switch();
        Switch(String alexaInvokeName, unsigned int port, CallbackFunction onCallback, CallbackFunction offCallback,CallbackFunction2 doorSensorCallback);
        ~Switch();
        String getAlexaInvokeName();
        void serverLoop();
        void respondToSearch(IPAddress& senderIP, unsigned int senderPort);
        void postbackDoorStatus();//IPAddress& requesterIP, String state
};
 
#endif
