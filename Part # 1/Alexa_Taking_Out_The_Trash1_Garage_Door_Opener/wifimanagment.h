#include <DNSServer.h>
#include <WiFiManager.h>
const char* ssid;
const char* password;

extern IPAddress ip;
extern IPAddress gateway;
extern IPAddress subnet;
boolean connectWifi();
byte value;
bool shouldSaveConfig = false;


//callback notifying us of the need to save config
void saveConfigCallback () {
        Serial.println("Should save config");
        shouldSaveConfig = true;
        Serial.println("connected...yeey :)");

        DynamicJsonBuffer jsonBuffer;
        JsonObject& root = jsonBuffer.createObject();
        JsonObject& params = root.createNestedObject("params");

        params["ap_name"] = WiFi.SSID();
        params["ap_password"] = WiFi.psk();
        char buf[400];
        params.printTo(buf);
        eepromWrite(buf,12);

}

boolean startAP(){
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

    if (!wifiManager.startConfigPortal("NetmediasAP")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.reset();
      delay(5000);
    }
    Serial.println("local ip");
    connectWifi();
}


boolean connectWifi(){
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root= jsonBuffer.parseObject(eepromRead(12,512));
  Serial.println("connectWifi()-->");
  //Serial.println(root.success());
   if (root.success()) {
      ssid = root["ap_name"];
      Serial.println(ssid);
      password = root["ap_password"];
      Serial.println(password);
   }else{
      return false;
   }

   //ssid     = "ssid name"; //test
  //password = "password";     //test
  boolean state = true;
  int i = 0;
  WiFi.config(ip, gateway, subnet);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  //MDNS.begin("garagedoor");
  Serial.println("");
  Serial.println("Connecting to WiFi");
 
  // Wait for connection
  Serial.print("Connecting ...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 10){
      state = false;
      break;
    }
    i++;
  }

  if (state){
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

  }
  else {

    Serial.println("");
    Serial.println("Connection failed.");
    return false;
  }
  return true;

}
