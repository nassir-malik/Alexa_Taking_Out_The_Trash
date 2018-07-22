extern int trigPinDoorOne;
extern int echoPinDoorOne;
extern int trigPinDoorTwo;
extern int echoPinDoorTwo;
extern int sensor_distance_from_door_one;
extern int sensor_distance_from_door_two;
int cduration, cdistance;

void doorSensorC(int trigPin, int echoPin);
void calibrateDoorOne();
void calibrateDoorTwo();

void calibrateDoors(){

  calibrateDoorOne();
  //calibrateDoorTwo();
}

void calibrateDoorOne(){
  doorSensorC(trigPinDoorOne, echoPinDoorOne);
  sensor_distance_from_door_one = cdistance;
  eepromWrite(String(cdistance),0);
  Serial.print("Door 1 Distance: ");
  Serial.println(cdistance);
  eepromRead(0,5);
}

void calibrateDoorTwo(){
  doorSensorC(trigPinDoorTwo, echoPinDoorTwo);
  sensor_distance_from_door_two =  cdistance;
  eepromWrite(String(cdistance),6);
    Serial.print("Door 2Distance: ");
  Serial.println(cdistance);
  eepromRead(6,11);
}


void doorSensorC(int trigPin, int echoPin){
      Serial.print("Calibrating....");

            digitalWrite(trigPin, LOW);
            delayMicroseconds(5);
            
            digitalWrite(trigPin, HIGH);
            delayMicroseconds(8);
            digitalWrite(trigPin, LOW);
            cduration = pulseIn(echoPin, HIGH);
            cdistance = (cduration/2) / 68; //Inches
            //distance = (duration/2) / 29.1; //centimeter
            Serial.println(cdistance);

      Serial.println("Calibration completed");

}
