
#include <SoftwareSerial.h>

// RX,  TX
SoftwareSerial xBeeSerial(A0,A1); //RX,TX

void setup() {
  Serial.begin(57600);

  while(!Serial){
    ;
  }

  Serial.println("USB Serial open...");
  
  xBeeSerial.begin(57600);

}

void loop() {
  
  if(xBeeSerial.available()>0){
    Serial.write(xBeeSerial.read());
    
  }
  if(Serial.available() > 0){
    xBeeSerial.write(Serial.read());
  }
 
}
