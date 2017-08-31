
#include <SoftwareSerial.h>

#define MAX_BUF_SIZE 32

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


  outputStream(xBeeSerial, Serial);

  outputStream(Serial,xBeeSerial);
 
}


void outputStream(Stream &s_in, Stream &s_out){

  uint8_t bytesToRead;
  bytesToRead = s_in.available();
  
  if(bytesToRead > 0){
    uint8_t *dataBuf;
    dataBuf = new uint8_t[min(bytesToRead,MAX_BUF_SIZE)];
    s_in.readBytes(dataBuf, bytesToRead);
    for(int i=0; i<bytesToRead; i++){
      s_out.write(dataBuf[i]);
    }
    delete [] dataBuf;  
  }
  
}

