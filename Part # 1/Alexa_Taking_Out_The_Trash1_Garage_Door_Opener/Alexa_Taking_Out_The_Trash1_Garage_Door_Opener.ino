#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <functional>
#include <WebSocketsServer.h>
#include "switch.h"
#include "UpnpBroadcastResponder.h"
#include "CallbackFunction.h"
#include "eeprom_read_write.h" 
#include "sensor.h"
#include "wifimanagment.h" 

extern "C"{
 #include "user_interface.h"
}
boolean connectWifi();
void garageDoorOneOpen();
void garageDoorOneClose();
void garageDoorTwoOpen();
void garageDoorTwoClose();
String doorSensorOne();
String doorSensorTwo();
String senderStateRequest;  
//IPAddress requesterIP;
//#######################################
// Update this before you flash
//chang it as per your network - windows open cmd and issue "ipconfig" command to get the info
IPAddress ip(192, 168, 0, 190); // use an avaible ip on your network
IPAddress gateway(192,168,0,1); // your router's ip  
IPAddress subnet(255,255,255,0);  // your router's subnet
//#######################################
// change gpio pins as needed.
//I am using NodeMcu V 0.1
const int relayPin1 = 16; // D0 - garageDoorOne
int trigPinDoorOne = 5; //D1
int echoPinDoorOne = 4; //D2

const int relayPin2 = 14; // D5 - garageDoorTwo
int trigPinDoorTwo = 12; //D6
int echoPinDoorTwo = 13; //D7

int duration, distance;
String garageDoorCurrState;
int sensor_distance_from_door_one = 0;
int sensor_distance_from_door_two = 0;


//#######################################
boolean wifiConnected = false;
UpnpBroadcastResponder upnpBroadcastResponder;
Switch *garageDoorOne = NULL;
Switch *garageDoorTwo = NULL;
WebSocketsServer webSocket(83);

void startWebSocket() { // Start a WebSocket server
  webSocket.begin();                          // start the websocket server
  webSocket.onEvent(webSocketEvent);          // if there's an incomming websocket message, go to function 'webSocketEvent'
  Serial.println("WebSocket server started.");
}
void setup()
{
    
   Serial.begin(115200);
   EEPROM.begin(512);
   
   pinMode(0, INPUT);
  
   pinMode(relayPin1, OUTPUT);
   pinMode(relayPin2, OUTPUT);
   
   pinMode(trigPinDoorOne, OUTPUT); // Sets the trigPin as an Output
   pinMode(echoPinDoorOne, INPUT); // Sets the echoPin as an Input
   
   pinMode(trigPinDoorTwo, OUTPUT); // Sets the trigPin as an Output
   pinMode(echoPinDoorTwo, INPUT); // Sets the echoPin as an Input
   
  // Initialise wifi connection
  wifiConnected = connectWifi();
  if(wifiConnected){
    startWebSocket();
    attachInterrupt(digitalPinToInterrupt(0), calibrateDoors, FALLING); //recaculate door open state once the flash button is pressed
    upnpBroadcastResponder.beginUdpMulticast();
    
    // Define your switches here. Max 14
    // Format: Alexa invocation name, local port no, on callback, off callback
    //http://192.168.0.190/gdoor?status=%20open                http://192.168.0.190/doorstatus
    garageDoorOne = new Switch("garage door one", 80, garageDoorOneOpen, garageDoorOneClose,doorSensorOne);
    //http://192.168.0.190:81/gdoor?status=%20open              http://192.168.0.190:81/doorstatus
    garageDoorTwo = new Switch("garage door two", 81, garageDoorTwoOpen, garageDoorTwoClose,doorSensorTwo);

    Serial.println("Adding switches upnp broadcast responder");
    upnpBroadcastResponder.addDevice(*garageDoorOne);
    upnpBroadcastResponder.addDevice(*garageDoorTwo);
    sensor_distance_from_door_one = eepromRead(0,5).toInt();  // intilize saved distance from memory - door one
    sensor_distance_from_door_two = eepromRead(6,11).toInt();// intilize saved distance from memory - door two
    Serial.print("Door One Distance: ");
    Serial.println(sensor_distance_from_door_one);
    Serial.print("Door Two Distance: ");
    Serial.println(sensor_distance_from_door_two);
  }
}
 
void loop()

{
  
 // Serial.print("inloop ...");
	if(WiFi.status() == WL_CONNECTED){
      upnpBroadcastResponder.serverLoop();
      garageDoorTwo->serverLoop();
      garageDoorOne->serverLoop();
      webSocket.loop();  
      
      if(senderStateRequest==doorSensorOne()){
        garageDoorOne->postbackDoorStatus();//requesterIP, senderStateRequest
     }
	 }else{
    Serial.print("loop start AP ...");
    startAP();
	 }

   delay(500);
}

void garageDoorOneOpen() {
    Serial.println("Open garage door 1 ...");
    //digitalWrite(relayPin1, HIGH);
    doorSensor(trigPinDoorOne,echoPinDoorOne,sensor_distance_from_door_one);
    //Serial.println("garageDoorCurrState=");
    //Serial.print(garageDoorCurrState);
    if(garageDoorCurrState == "close"){toggleRelay(relayPin1);webSocket.broadcastTXT(""+String(sensor_distance_from_door_one)+","+garageDoorCurrState);}
}

void garageDoorOneClose() {
    Serial.print("Close garage door 1 ...");
    //digitalWrite(relayPin1, LOW);
    doorSensor(trigPinDoorOne,echoPinDoorOne,sensor_distance_from_door_one);
    if(garageDoorCurrState == "open"){toggleRelay(relayPin1);}
}

String doorSensorOne() {
    doorSensor(trigPinDoorOne,echoPinDoorOne,sensor_distance_from_door_one);
    return garageDoorCurrState;
}

void garageDoorTwoOpen() {
    Serial.print("Open garage door 2 ...");
    //digitalWrite(relayPin2, HIGH);
    doorSensor(trigPinDoorTwo,echoPinDoorTwo,sensor_distance_from_door_two);
    if(garageDoorCurrState == "close"){toggleRelay(relayPin2);}
}

void garageDoorTwoClose() {
  Serial.print("Close garage door 2 ...");
  //digitalWrite(relayPin2, LOW);
  doorSensor(trigPinDoorTwo,echoPinDoorTwo,sensor_distance_from_door_two);
  if(garageDoorCurrState == "open"){toggleRelay(relayPin2);}
}
String doorSensorTwo() {
    doorSensor(trigPinDoorTwo,echoPinDoorTwo,sensor_distance_from_door_two);
    return garageDoorCurrState;
}

void toggleRelay(int pinId){
  digitalWrite(pinId, HIGH);
  delay(1000);
  digitalWrite(pinId, LOW);
}
void doorSensor(int trigPin, int echoPin, int sensor_distance_from_door){
            digitalWrite(trigPin, LOW);
            delayMicroseconds(5);
            
            digitalWrite(trigPin, HIGH);
            delayMicroseconds(8);
            digitalWrite(trigPin, LOW);
            duration = pulseIn(echoPin, HIGH);
            distance = (duration/2) / 70; //Inches
            //distance = (duration/2) / 29.1; //centimeter
            //Serial.println(distance);
            if (distance <= sensor_distance_from_door ){
                  //Serial.println("distance=");
                  //Serial.print(distance);
              garageDoorCurrState = "open";
            }else{
                  //Serial.println("distance=");
                  //Serial.print(distance);
              garageDoorCurrState = "close";
            }
}



void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) { // When a WebSocket message is received
  switch (type) {
    case WStype_DISCONNECTED:             // if the websocket is disconnected
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED:  {           // if a new websocket connection is established
        webSocket.sendTXT(num,"WS Connected...");
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        //rainbow = false;                  // Turn rainbow off when a new connection is established
    }
      break;
    case WStype_TEXT:                     // if new text data is received
      Serial.printf("[%u] Got Text: %s\n", num, payload);
      if(String((char*)payload).indexOf("calibrate")>-1){
        Serial.println("Execution calibration command...");calibrateDoors();
        
        }
        doorSensor(trigPinDoorOne,echoPinDoorOne,sensor_distance_from_door_one);
        webSocket.sendTXT(num,""+String(sensor_distance_from_door_one)+","+garageDoorCurrState);
      break;
  }
}
