#include <SoftwareSerial.h>

#define CAMBUFFSIZE 100

SoftwareSerial camSerial = SoftwareSerial(18, 19);

uint8_t serialNum = 0;
uint8_t cmd = 0x11;

uint8_t camerabuff[CAMBUFFSIZE+1];
uint8_t bufferLen = 0;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  camSerial.begin(38400);
}

void loop() {
  /*
    while (Serial.available())
      camSerial.write(Serial.read());
  while (camSerial.available())
    Serial.write(camSerial.read());
    */

    camSerial.write((byte)0x56);
    camSerial.write((byte)serialNum);
    camSerial.write((byte)cmd);
    camSerial.write((byte)0x00);

    int timeout = 5;
    int buffpos = 0;
    delay(200);
    if(!readResponse(CAMBUFFSIZE,200)){
      Serial.println("Read Response: Nothing received");
    }
    else
      Serial.println("Received response!");

    delay(1000);
    
}


uint8_t readResponse(uint8_t numbytes, uint8_t timeout) {
  uint8_t counter = 0;
  bufferLen = 0;
  int avail;
 
  while ((timeout != counter) && (bufferLen != numbytes)){

    avail = camSerial.available();

    if (avail <= 0) {
      delay(1);
      counter++;
      continue;
    }
    counter = 0;
    // there's a byte!

    camerabuff[bufferLen++] = (byte)camSerial.read();

  }
  //printBuff();
  camerabuff[bufferLen] = 0;
  Serial.print("Rx: ");
  Serial.println((char*)camerabuff);
  Serial.print("Buffer Len: ");
  Serial.println(bufferLen);
  //camSerial.flush();
  return bufferLen;
}
