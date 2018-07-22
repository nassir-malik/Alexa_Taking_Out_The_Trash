#include <EEPROM.h>
#include <ArduinoJson.h>

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//Write data to memory
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

void eepromWrite(String jsonData,int pos)
{
  Serial.println("writing to eeprom...");
  //Serial.println(pos);
  Serial.print(jsonData);
  int z=0;
  
  for(int i=0; i<jsonData.length(); i++) //write value to eeprom
  {
    
    EEPROM.write(i+pos, jsonData[i]); 
    z=i+pos;
  }
  EEPROM.write(z+1,'\0');
  EEPROM.commit();
}
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//read saved data from memory
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

String eepromRead(int pos, int len){
  Serial.println("Reading from EEPROM...");
  //Serial.println(pos);
  String savedvalue="";
  for (int i = pos; i <len; ++i)
    {
      savedvalue += char(EEPROM.read(i));
    }
    Serial.print(savedvalue);
    return savedvalue;
}
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

