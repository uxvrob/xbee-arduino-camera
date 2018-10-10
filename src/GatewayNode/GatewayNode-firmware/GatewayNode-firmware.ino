/********************************************************************************
 * 
    Copyright (c) 2017-2018 Robbie Sharma. All rights reserved

    This file is part of xbee-arduino-camera.
    <https://github.com/uxvrob/xbee-arduino-camera>

    xbee-arduino-camera is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    xbee-arduino-camera is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with xbee-arduino-camera.  If not, see <https://www.gnu.org/licenses/>.
    
********************************************************************************/

/*******************************************************************************
* GatewayNode-Firmware.ino
*
* @author Robbie Sharma <robbie@rsconsulting.ca>
* @date September 16, 2017
* @desc Arduino firmware for Wireless SD shield interfacing with xBee as a
* Gateway for CameraNodes
* 
*******************************************************************************/

#include <XBee.h>
#include <Printers.h>
#include <SoftwareSerial.h>

#define CAMERA_SH_ADDR 0x13A200
#define CAMERA_SL_ADDR 0x415B894A

XBeeWithCallbacks xbee;


char cmdBuf[25];
uint8_t cmdidx = 0;
bool cmdComplete =false;



SoftwareSerial xss = SoftwareSerial(A0,A1);

void zbCallback(ZBRxResponse& rx, uintptr_t){

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

    xbee.loop();

   if(cmdComplete){
      
      XBeeAddress64 addr64 = XBeeAddress64(CAMERA_SH_ADDR, CAMERA_SL_ADDR);
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
