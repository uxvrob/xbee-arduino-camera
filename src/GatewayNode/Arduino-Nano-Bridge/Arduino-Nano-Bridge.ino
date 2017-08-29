
#include <SoftwareSerial.h>

// RX,  TX
SoftwareSerial brg(8,9);

void setup() {
  Serial.begin(57600);

  while(!Serial){
    ;
  }

  Serial.println("Success");
  
  brg.begin(115200);

}

void loop() {
  
  if(brg.available()>0){
    Serial.write((uint8_t)brg.read());
  }
  if(Serial.available() > 0){
    brg.write((uint8_t)Serial.read());
  }
 
}
