
/***************************************************************
* Gateway Node firmware
*
* @author Robbie Sharma robbie -at- rsconsulting.ca
* @date August 21, 2017
* @desc Before upload, flip switch to "USB" on the arduino Shield
* 
*  TODO:  Develop a simple protocol framework with handshaking
*  for transmitting data between xBee modules and Server. 
***************************************************************/

#include <SoftwareSerial.h>

// Incresae software serial maximum buffer size 
#define _SS_MAX_RX_BUFF 256
#define MAX_BUF_SIZE 64

// RX,  TX
SoftwareSerial xBeeSerial(A0,A1); //RX,TX

EasyTransfer EIn, EOut;

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
  delay(10);
  outputStream(Serial,xBeeSerial);
  delay(10);
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

