#include "Switch.h"
#include "CallbackFunction.h"
IPAddress requesterIP; 
extern String senderStateRequest; 
String html ="<!DOCTYPE html><html><head> <title>Netmedias' Grage Door openerner</title> <link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css\" integrity=\"sha384-BVYiiSIFeK1dGmJRAkycuHAHRg32OmUcww7on3RYdg4Va+PmSTsz/K68vbdEjh4u\" crossorigin=\"anonymous\"> <meta name=\"theme-color\" content=\"#00878f\"> <meta content='width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=0' name='viewport'> <script> var rainbowEnable = false; var connection = new WebSocket('ws://192.168.0.190:83/', ['arduino']); connection.onopen = function () { connection.send('Connect ' + new Date()); }; connection.onerror = function (error) { console.log('WebSocket Error ', error); }; connection.onmessage = function (e) { document.getElementById(\"one\").value = e.data; console.log('Server: ', e.data); }; connection.onclose = function () { console.log('WebSocket connection closed'); }; function calibrateDoors () { document.getElementById(\"one\").value =\"Calibrating Garage Door\"; console.log('Calibarting Garage Door'); connection.send(\"calibrate\"); } </script> </head> <body> <center> <header> <h1>Netmedias' Grage Door Opener</h1> </header> <div> <table> <tr> <td text-align: right\">Garage Door One </td> <td><input class=\"enabled\" id=\"one\" type=\"text\" ></td> </tr> <tr> <td text-align: right\">Garage Door Two</td> <td><input class=\"enabled\" id=\"two\" type=\"text\" value=\"Not connected\"></td> </tr> </table> <p style=\"margin:8px 0px\"> <button id=\"Calibrate\" class=\"btn btn-primary\" type=\"submit\" style=\"background-color:#999\" onclick=\"calibrateDoors();\">Calibrate Garage Doors</button> </p> </div> </center></body></html>";

//<<constructor>>
Switch::Switch(){
    Serial.println("default constructor called");
}
//Switch::Switch(String alexaInvokeName,unsigned int port){
Switch::Switch(String alexaInvokeName, unsigned int port, CallbackFunction oncb, CallbackFunction offcb,CallbackFunction2 doorSensor){
    uint32_t chipId = ESP.getChipId();
    char uuid[64];
    sprintf_P(uuid, PSTR("38323636-4558-4dda-9188-cda0e6%02x%02x%02x"),
          (uint16_t) ((chipId >> 16) & 0xff),
          (uint16_t) ((chipId >>  8) & 0xff),
          (uint16_t)   chipId        & 0xff);
    
    serial = String(uuid);
    persistent_uuid = "Socket-1_0-" + serial+"-"+ String(port);
        
    device_name = alexaInvokeName;
    localPort = port;
    onCallback = oncb;
    offCallback = offcb;
    doorSensorCallback =doorSensor;
    startWebServer();
}


 
//<<destructor>>
Switch::~Switch(){/*nothing to destruct*/}

void Switch::serverLoop(){
    if (server != NULL) {
        server->handleClient();
        delay(1);
    }
}

void Switch::startWebServer(){
  server = new ESP8266WebServer(localPort);

  server->on("/", [&]() {
    handleRoot();
  });
 

  server->on("/setup.xml", [&]() {
    handleSetupXml();
  });
  
  server->on("/index.html", [&]() {
      String response = html;//"<html><body><h1>It works!</h1></body></html>";
       server->send(200, "text/html", response.c_str());
  });
  
  server->on("/upnp/control/basicevent1", [&]() {
    handleUpnpControl();
  });


  server->on("/gdoor", [&]() {
    handleOnOff();
  });
  
  server->on("/doorstatus", [&]() {
    doorStatus();
  });
  server->on("/eventservice.xml", [&]() {
    handleEventservice();
  });

  //server->onNotFound(handleNotFound);
  server->begin();

  Serial.print("WebServer started on port: ");
  Serial.println(localPort);
}
 
void Switch::handleEventservice(){
  Serial.println(" ########## Responding to eventservice.xml ... ########\n");
 
  String eventservice_xml = "<?scpd xmlns=\"urn:Belkin:service-1-0\"?>"
        "<actionList>"
          "<action>"
            "<name>SetBinaryState</name>"
            "<argumentList>"
              "<argument>"
                "<retval/>"
                "<name>BinaryState</name>"
                "<relatedStateVariable>BinaryState</relatedStateVariable>"
                "<direction>in</direction>"
              "</argument>"
            "</argumentList>"
             "<serviceStateTable>"
              "<stateVariable sendEvents=\"yes\">"
                "<name>BinaryState</name>"
                "<dataType>Boolean</dataType>"
                "<defaultValue>0</defaultValue>"
              "</stateVariable>"
              "<stateVariable sendEvents=\"yes\">"
                "<name>level</name>"
                "<dataType>string</dataType>"
                "<defaultValue>0</defaultValue>"
              "</stateVariable>"
            "</serviceStateTable>"
          "</action>"
        "</scpd>\r\n"
        "\r\n";
          
    server->send(200, "text/plain", eventservice_xml.c_str());
}
 
void Switch::handleUpnpControl(){
  Serial.println("########## Responding to  /upnp/control/basicevent1 ... ##########");      
  
 for (int x=0; x <= server->args(); x++) {
  Serial.println(server->arg(x));
  }
  //WiFiClient client;
  String request = server->arg(0);      
  Serial.print("ip:");
  //IPAddress ip= WiFiClient.remoteIP();//.toString().c_str();
  IPAddress senderIP = server->client().remoteIP();
  Serial.println(senderIP);

  if(request.indexOf("<BinaryState>1</BinaryState>") > 0) {
      Serial.println("Got Turn on request");
      onCallback();
  }

  if(request.indexOf("<BinaryState>0</BinaryState>") > 0) {
      Serial.println("Got Turn off request");
      offCallback();
  }
  
  server->send(200, "text/plain", "");
}
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
void Switch::handleOnOff(){
  Serial.println("########## Responding to  /gdoor?status=%20open ... ##########");      
  
 for (int x=0; x <= server->args(); x++) {
  Serial.println(server->arg(x));
  }
  requesterIP = server->client().remoteIP();
  String request = server->arg(0);      
  Serial.print("+senderIP:");
  Serial.println(requesterIP);

  if(request.indexOf("open") > 0) {
      Serial.println("Got open request");
      senderStateRequest = "open";
      onCallback();
  }

  if(request.indexOf("close") > 0) {
      Serial.println("Got close request");
      senderStateRequest = "close";
      offCallback();
  }
  
  server->send(200, "text/plain", "");
}
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
void Switch::doorStatus(){
  Serial.println("########## Responding to  /doorstatus ... ##########");      
  String val=doorSensorCallback();
  server->send(200, "text/plain", val);
}
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
void Switch::postbackDoorStatus(){//IPAddress& requesterIP, String state
    WiFiClient client;
   String PostData = "state=open";
    
    if (client.connect(requesterIP, 80)) {
      Serial.println("connected");
      client.println("POST /doorstate HTTP/1.1");
      client.println("Host:  artiswrong.com");
      client.println("User-Agent: Arduino/1.0");
      client.println("Connection: close");
      client.println("Content-Type: application/x-www-form-urlencoded;");
      client.print("Content-Length: ");
      client.println(PostData.length());
      client.println();
      client.println(PostData);
    senderStateRequest = "none";
  } else {
    Serial.println("connection failed");
  }

  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
  }
  
}
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
void Switch::handleRoot(){
  server->send(200, "text/plain", "You should tell Alexa to discover devices");
}

void Switch::handleSetupXml(){
  Serial.println(" ########## Responding to setup.xml ... ########\n");
  
  IPAddress localIP = WiFi.localIP();
  char s[16];
  sprintf(s, "%d.%d.%d.%d", localIP[0], localIP[1], localIP[2], localIP[3]);
  Serial.println("localiP:");
  Serial.println(WiFi.localIP());
  String setup_xml = "<?xml version=\"1.0\"?>"
        "<root>"
         "<device>"
            "<deviceType>urn:Belkin:device:controllee:1</deviceType>"
            "<friendlyName>"+ device_name +"</friendlyName>"
            "<manufacturer>Belkin International Inc.</manufacturer>"
            "<modelName>Emulated Socket</modelName>"
            "<modelNumber>3.1415</modelNumber>"
            "<UDN>uuid:"+ persistent_uuid +"</UDN>"
            "<serialNumber>221517K0101769</serialNumber>"
            "<binaryState>0</binaryState>"
            "<serviceList>"
              "<service>"
                  "<serviceType>urn:Belkin:service:basicevent:1</serviceType>"
                  "<serviceId>urn:Belkin:serviceId:basicevent1</serviceId>"
                  "<controlURL>/upnp/control/basicevent1</controlURL>"
                  "<eventSubURL>/upnp/event/basicevent1</eventSubURL>"
                  "<SCPDURL>/eventservice.xml</SCPDURL>"
              "</service>"
          "</serviceList>" 
          "</device>"
        "</root>\r\n"
        "\r\n";
        
    server->send(200, "text/xml", setup_xml.c_str());
    
    Serial.print("Sending :");
    Serial.println(setup_xml);
}

String Switch::getAlexaInvokeName() {
    return device_name;
}

void Switch::respondToSearch(IPAddress& senderIP, unsigned int senderPort) {
  Serial.println("");
  Serial.print("Sending response to ");
  Serial.println(senderIP);
  Serial.print("Port : ");
  Serial.println(senderPort);

  IPAddress localIP = WiFi.localIP();
  char s[16];
  sprintf(s, "%d.%d.%d.%d", localIP[0], localIP[1], localIP[2], localIP[3]);

  String response = 
       "HTTP/1.1 200 OK\r\n"
       "CACHE-CONTROL: max-age=86400\r\n"
       "DATE: Sat, 26 Nov 2016 04:56:29 GMT\r\n"
       "EXT:\r\n"
       "LOCATION: http://" + String(s) + ":" + String(localPort) + "/setup.xml\r\n"
       "OPT: \"http://schemas.upnp.org/upnp/1/0/\"; ns=01\r\n"
       "01-NLS: b9200ebb-736d-4b93-bf03-835149d13983\r\n"
       "SERVER: Unspecified, UPnP/1.0, Unspecified\r\n"
       "ST: urn:Belkin:device:**\r\n"
       "USN: uuid:" + persistent_uuid + "::urn:Belkin:device:**\r\n"
       "X-User-Agent: redsonic\r\n\r\n";

  UDP.beginPacket(senderIP, senderPort);
  UDP.write(response.c_str());
  UDP.endPacket();                    

   Serial.println("Response sent !");
}


