#include <XBee.h>
#include <Printers.h>

#define SER_BAUD_RATE 57600         // Serial baud rate
#define GATEWAY_SH_ADDR 0x0013A200
#define GATEWAY_SL_ADDR 0x415B8949

uint8_t payload[] = { 3, 2 };

XBee xbee = XBee();
XBeeAddress64 addr64 = XBeeAddress64(GATEWAY_SH_ADDR, GATEWAY_SL_ADDR);
ZBTxRequest zbTx = ZBTxRequest(addr64, payload, sizeof(payload));
ZBTxStatusResponse txStatus = ZBTxStatusResponse();


void setup() {

  Serial.begin(SER_BAUD_RATE);
  xbee.setSerial(Serial);

}


void loop(){
  xbee.send(zbTx);

  // flash TX indicator
  //flashLed(statusLed, 1, 100);

  // after sending a tx request, we expect a status response
  // wait up to half second for the status response
  if (xbee.readPacket(500)) {
    // got a response!

    // should be a znet tx status              
    if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
      xbee.getResponse().getZBTxStatusResponse(txStatus);

      // get the delivery status, the fifth byte
      if (txStatus.getDeliveryStatus() == SUCCESS) {
        // success.  time to celebrate
        //flashLed(statusLed, 5, 50);
      } else {
        // the remote XBee did not receive our packet. is it powered on?
        //flashLed(errorLed, 3, 500);
      }
    }
  } else if (xbee.getResponse().isError()) {
    //nss.print("Error reading packet.  Error code: ");  
    //nss.println(xbee.getResponse().getErrorCode());
  } else {
    // local XBee did not provide a timely TX Status Response -- should not happen
    //flashLed(errorLed, 2, 50);
  }

  delay(1000);


}


/*
void loop(){
    

    // Command state machine
    // If a command string terminated by \n is received, process the string

    image_file_t* ift;
    ift = new image_file_t[1];
    
    
    if(cmdComplete){

      cmdBuf.trim();
      
      if(cmdBuf.equals("AV+CGETS")){        // Send snapshot via Serial routine

        //takeSnapshotTransmitSaveToSD(ift);


      }else if(cmdBuf.equals("AV+SGETS")){        // Send snapshot via Serial routine
        
        camNode.takeSnapshotSaveToSD(ift);
        //sendSnapshotFile(ift->szName);

      }else if(cmdBuf.equals("AV+SNAP")){
        
        camNode.takeSnapshotSaveToSD(ift);
        
      }else if(cmdBuf.equals("AV+RECF")){
        if(camNode.getRecentImageFilename(ift->szName)){

          File f = SD.open(ift->szName, FILE_READ);
          Serial.print(F("Recent: "));
          Serial.print(ift->szName);
          Serial.print(F(" Size: "));
          Serial.println(f.size());
          f.rewindDirectory();
          f.close();
        }
        else
          Serial.println(F("No file created yet"));
        
      }else if(cmdBuf.equals("AV+FILES")){
        
        File root = SD.open("/");
        root.rewindDirectory();
        camNode.printDirectory(root, 0);
        root.rewindDirectory();
        root.close();
      
      }else if(cmdBuf.equals("AV+CSEND")){
        
        camNode.getRecentImageFilename(ift->szName);
        //sendSnapshotFile(ift->szName);
      
      }else if(cmdBuf.equals("AV+DEBUGON")){

        camNode.debugOn();
      
      }else if(cmdBuf.equals("AV+JRES")){   // Command to reset the arduino.  NOT IMPLEMENTED YET.
      
        Serial.println(F("ROK"));
      
      }else if(cmdBuf.equals("OK")){        // ACK type command
        
          
      }else{                                // Invalid CMD received
        //Serial.print("NOK: ");
        //Serial.println(cmdBuf);
        //Serial.write('\n');
      }

      // Reset command buffers
      cmdBuf = "";
      cmdComplete = false;
  }

  processSerial();

  delete [] ift;
}

// Thread for processing serial events.  A byte or char is added to a buffer until 
// a newline character is received.

void processSerial(){

  int bytesAvail = Serial.available();

  if(bytesAvail > 0){
  
    for(int i = 0; i < bytesAvail; i++){
      
      char inChar = (char)Serial.read();
      cmdBuf += inChar;
  
      if(inChar == '\n'){
        cmdComplete = true;
      }
      
    }
  }
  
}

*/
