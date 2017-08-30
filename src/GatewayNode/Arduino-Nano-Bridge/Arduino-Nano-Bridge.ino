
#include <SoftwareSerial.h>

// RX,  TX
SoftwareSerial brg(A0,A1); //RX,TX

void setup() {
  Serial.begin(57600);

  while(!Serial){
    ;
  }

  Serial.println("USB Serial open...");
  
  brg.begin(57600);

}

void loop() {
  
  if(brg.available()>0){
    Serial.write(brg.read());
    
  }
  if(Serial.available() > 0){
    brg.write(Serial.read());
  }
 
}
