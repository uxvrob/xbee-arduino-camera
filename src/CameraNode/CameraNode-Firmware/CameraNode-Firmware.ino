#include "Node.h"
#include "CameraNode.h"

#define CAM_RX_PIN 2
#define CAM_TX_PIN 3

#define SER_BAUD_RATE 57600         // Serial baud rate

#define GATEWAY_SH_ADDR 0x0013A200
#define GATEWAY_SL_ADDR 0x415B8949

//#define DEBUG 1


bool cmdComplete = false;

String cmdBuf = "";

CameraNode camNode = CameraNode(CAM_RX_PIN, CAM_TX_PIN);

void _zbRxCb(ZBRxResponse& rx, uintptr_t){
	
  camNode.freeRam();
  String cmdBuf = "";
  
  bool activityError = false;
  
  for(uint8_t i=0; i < rx.getDataLength(); i++)
    cmdBuf +=(char)rx.getData()[i];

  cmdBuf.trim();

  if(cmdBuf.equals("AV+SGETS")){        // Save snapshot to SD and transmit image
    if(!camNode.SDSaveActive || !camNode.transferActive){
		camNode.takeSnapshotSaveToSD();
		camNode.sendSnapshotFile(camNode._ift.szName);
	}
	else
		activityError = true;
	

  }else if(cmdBuf.equals("AV+SNAP")){  // Save snapshot to SD
    if(!camNode.SDSaveActive)
		camNode.takeSnapshotSaveToSD();
	else
		activityError = true;
    
  }else if(cmdBuf.equals("AV+RECF")){	// Print recent image filename and size
	
    if(camNode.getRecentImageFilename(camNode._ift.szName)){

	  
      File f = SD.open(camNode._ift.szName, FILE_READ);

      Serial.print(F("Recent: "));
      Serial.print(camNode._ift.szName);
	  camNode.sendPayload(camNode._ift.szName);
      Serial.print(F(" Size: "));
      Serial.println(f.size());
      f.rewindDirectory();
      f.close();

    }
    else
      Serial.println(F("No file created yet"));
  }else if(cmdBuf.equals("AV+CSEND")){	// Transmit recently saved image
    if(!camNode.transferActive){
		camNode.getRecentImageFilename(camNode._ift.szName);
		camNode.sendSnapshotFile(camNode._ift.szName);
	}
	else
		activityError = true;
	
  }else if(cmdBuf.equals("AV+JRES")){   // Print ROK
  // Command to reset the arduino.  NOT IMPLEMENTED YET.
  
    Serial.println(F("ROK"));
  
  }else if(cmdBuf.indexOf(',') != -1){  // Further parsing of commands with ',' delimeters
	
	// RX: {+,<packet_index>;}
	int tknIdx = cmdBuf.indexOf(',');
	
	if(cmdBuf.substring(0,tknIdx).equals('+')){
		
		camNode._ift.uPacketIndex = cmdBuf.substring(cmdBuf.indexOf(',',tknIdx+1)).toInt();
		
	}
	
  }
  else{
	  
	  // Unrecognized command
	  Serial.println(F("AV+ERR,NOTREC"));
	  camNode.sendPayload(F("AV+ERR,cmdUnrecognized"));
  }
  
  if(activityError)
	camNode.sendPayload(F("AV+ERR,activeprocess"));

  // Reset command buffers
  cmdBuf = "";
  camNode.freeRam();
}

void setup() {

  Serial.begin(SER_BAUD_RATE);
  camNode.setXbeeSerial(Serial);
  camNode.setRxAddress(GATEWAY_SH_ADDR,GATEWAY_SL_ADDR);
  camNode.setReceiveCb(_zbRxCb);
  camNode.begin();
  camNode.freeRam();
}

void loop(){
  camNode.spin();
}
