#include "Node.h"
#include "CameraNode.h"

#define CAM_RX_PIN 2
#define CAM_TX_PIN 3
#define SER_BAUD_RATE 57600         // Serial baud rate

bool cmdComplete = false;
String cmdBuf = "";

CameraNode camNode = CameraNode(CAM_RX_PIN, CAM_TX_PIN);

void setup() {

  Serial.begin(SER_BAUD_RATE);
  //camNode.setSerial(&Serial);
  camNode.begin();

}

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
