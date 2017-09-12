#include <XBee.h>
#include <Printers.h>
#include <SoftwareSerial.h>      
#include <Time.h>

XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
// create reusable response objects for responses we expect to handle 
ZBRxResponse rx = ZBRxResponse();
ModemStatusResponse msr = ModemStatusResponse();

char cmdBuf[25];
uint8_t cmdidx = 0;
bool cmdComplete =false;

uint32_t _msb = 0x13A200;
uint32_t _lsb = 0x415B894A;


SoftwareSerial xss = SoftwareSerial(A0,A1);

void setup() {
  Serial.begin(57600);
  while(!Serial){
    
  }
  xbee.setSerial(xss);
  xss.begin(57600);

  Serial.println("Gateway started");
}

void loop() {
  // put your main code here, to run repeatedly:

   if(cmdComplete){

     
      
      XBeeAddress64 addr64 = XBeeAddress64(_msb, _lsb);
      ZBTxRequest zbTx = ZBTxRequest(addr64, cmdBuf, cmdidx);
      ZBTxStatusResponse txStatus = ZBTxStatusResponse();

      xbee.send(zbTx);

      xbeeReadPacket();

       // Reset command buffers
      cmdidx = 0;
      cmdComplete = false;
  }

  processSerial();
}

void xbeeReadPacket(){

    xbee.readPacket();
    
    if (xbee.getResponse().isAvailable()) {
      // got something
           
      if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
        // got a zb rx packet
        
        // now fill our zb rx class
        xbee.getResponse().getZBRxResponse(rx);
        //Serial.print("payload [");
         for (int i = 0; i < rx.getDataLength()-3; i++) {
          /*
          Serial.print(i, DEC);
          Serial.print("] is ");
          */
          Serial.print((char)rx.getData()[i]);
        }
        //Serial.println("]");
        /*
       Serial.print("FrameData [");
       for (int i = 0; i < xbee.getResponse().getFrameDataLength(); i++) {
        //Serial.print("frame data [");
        //Serial.print(i, DEC);
        //Serial.print("] is ");
        Serial.print((char)xbee.getResponse().getFrameData()[i]);
       }
       Serial.println("]");
      */
      }
    } else if (xbee.getResponse().isError()) {
      Serial.print("error code:");
      Serial.println(xbee.getResponse().getErrorCode());
    }
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
