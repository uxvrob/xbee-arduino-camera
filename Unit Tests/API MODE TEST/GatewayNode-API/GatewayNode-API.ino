#include <XBee.h>
#include <Printers.h>
#include <SoftwareSerial.h>      

XBeeWithCallbacks xbee;


//XBeeResponse response = XBeeResponse();
// create reusable response objects for responses we expect to handle 
//ZBRxResponse rx = ZBRxResponse();

char cmdBuf[25];
uint8_t cmdidx = 0;
bool cmdComplete =false;

uint32_t _msb = 0x13A200;
uint32_t _lsb = 0x415B894A;


SoftwareSerial xss = SoftwareSerial(A0,A1);

void zbCallback(ZBRxResponse& rx, uintptr_t){
	/*
	ZBTxRequest tx;
	
	tx.setAddress64(rx.getRemoteAddress64());
	tx.setPayload(rx.getFrameData() + rx.getDataOffset(), rx.getDataLength());
	
	xbee.send(tx);
	*/
    for(uint8_t i=0; i < rx.getDataLength(); i++){
      Serial.write((char)rx.getData()[i]);
    }
	
	Serial.flush();
	
}

void setup() {
  Serial.begin(57600);
  while(!Serial){
    
  }

  //xbee.onPacketError(printErrorCb, (uintptr_t)(Print*)&Serial);
  //xbee.onTxStatusResponse(printErrorCb, (uintptr_t)(Print*)&Serial);
  //xbee.onZBTxStatusResponse(printErrorCb, (uintptr_t)(Print*)&Serial);
  xbee.onZBRxResponse(zbCallback);
  
  xbee.setSerial(xss);
  xss.begin(57600);

  Serial.println("Gateway started");
}

void loop() {
  // put your main code here, to run repeatedly:

    xbee.loop();

   if(cmdComplete){
      
      XBeeAddress64 addr64 = XBeeAddress64(_msb, _lsb);
      ZBTxRequest zbTx = ZBTxRequest(addr64, cmdBuf, cmdidx);
      ZBTxStatusResponse txStatus = ZBTxStatusResponse();

      xbee.sendAndWait(zbTx,500);
	  
       // Reset command buffers
      cmdidx = 0;
      cmdComplete = false;
  }

  processSerial();
}




void processSerial(){

  int bytesAvail = Serial.available();

  if(bytesAvail > 0){
  
    for(int i = 0; i < bytesAvail; i++){
      
      char inChar = (char)Serial.read();
      cmdBuf[cmdidx++] = inChar;

      if(cmdidx >= sizeof(cmdBuf))
        cmdidx=0;
  
      if(inChar == '\n'){
        cmdComplete = true;
      }
      
    }
  }
  
}
