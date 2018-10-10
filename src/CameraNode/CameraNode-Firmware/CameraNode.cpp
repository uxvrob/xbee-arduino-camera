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

/***************************************************************
* CameraNode.cpp
*
* @author Robbie Sharma <robbie@rsconsulting.ca>
* @date September 16, 2017
* @desc Implement to take images, save to SD, and transmit via xBee-Serial
 bridge.  Later, API commands will be used.
* 
***************************************************************/

#include "Node.h"
#include "CameraNode.h"


CameraNode::CameraNode(int cam_rx_pin, int cam_tx_pin):
  _nd(this),
  _camConn(new SoftwareSerial(cam_rx_pin, cam_tx_pin)),
  _cam(new Adafruit_VC0706(this->_camConn)),
  transferActive(false),
  SDSaveActive(false)
{
	
}

void CameraNode::begin(){
	
   _nd->begin();
  
    // Try to locate the camera
  if (_cam->begin()) {
    Serial.println(F("AV+CS,0x01;"));
	_nd->sendPayload("AV+CS,0x01;");
  } else {
    Serial.println(F("AV+CS,0x00;"));
	_nd->sendPayload("AV+CS,0x00;");
    return;
  }

  // Set the picture size - you can choose one of 640x480, 320x240 or 160x120 
  // Remember that bigger pictures take longer to transmit!
  
  _cam->setImageSize(VC0706_640x480);        // biggest
  //_cam->setImageSize(VC0706_160x120);          // small

}

void CameraNode::takeSnapshotSaveToSD(){

  // Get snapshot
  if (!_cam->takePicture()) {
    Serial.println(F("AV+ERR,TAKE_PIC;"));
    return;
  }

  // Create an image with incremental name IMAGExx.JPG
  
  
  if(!generateImageFilename(_ift.szName)){
    Serial.println(F("AV+ERR,GEN_FILENAME;"));
    return;
  }
    
  _ift.uSize = _cam->frameLength(); // Get the size of the image (frame) taken
  _ift.uPackets = _nd->convertFileSizeToPackets(_ift.uSize);
  _ift.uPacketIndex=0;

  
  #ifdef DEBUG 
	Serial.print(F("AV+DEBUG,TAKESNAP_SD,"));
	Serial.print(_ift.szName);
	Serial.print(F(","));
	Serial.print(_ift.uSize);
	Serial.print(F(","));
	Serial.print(_ift.uPackets);
	Serial.print(F(","));
	Serial.print(_ift.uPacketIndex);
	Serial.println(F(";"));
  #endif
  
  File imgFile = SD.open(_ift.szName, FILE_WRITE);  // file object for image storage
  
  _nd->freeRam();
  
  if(!imgFile) Serial.println(F("AV+ERR,FILE_OPEN;"));
  
  SDSaveActive = true;

  char* cbuf;
  cbuf = new uint8_t[MAX_BUF_SIZE];
  
  while (_ift.uPacketIndex < _ift.uPackets && imgFile){
	
    // read IMG_MAX_BUF_SIZE bytes at a time;
	
    uint16_t bytesToRead = (_ift.uPacketIndex == _ift.uPackets-1)?(_ift.uSize - (_ift.uPacketIndex*MAX_BUF_SIZE)):MAX_BUF_SIZE;
 
	   uint8_t* buf = _cam->readPicture(bytesToRead);
	   imgFile.write(buf, bytesToRead);
	   _ift.uPacketIndex++;


    if(_ift.uPacketIndex % 12 == 0){
      uint16_t p_size = sprintf(cbuf,"AV+SDSAVE,%u,%u;",_ift.uPacketIndex,_ift.uPackets);
      _nd->sendPayload(cbuf, p_size);
    }

  }
  
  SDSaveActive = false;
  imgFile.flush();
  imgFile.close();  

  delete [] cbuf;

  _cam->resumeVideo();

}


void CameraNode::sendSnapshotFile(char* filename){

  if(!SD.exists(filename)){
    Serial.println(F("AV+ERR,FILE_NO_EXIST;"));
    return;
  }
  
  File imgFile = SD.open(filename, FILE_READ);  // file object for image storage
  
  strcpy(_ift.szName,filename);
  _ift.uSize = imgFile.size();
  _ift.uPackets = _nd->convertFileSizeToPackets(_ift.uSize);
  _ift.uPacketIndex = 0;
  
  if(!imgFile){
    Serial.println(F("AV+ERR,FILE_OPEN;"));
    imgFile.close();
    return;
  }
  
  Serial.print(F("File size: "));
  Serial.println(_ift.uSize);
  
  uint8_t* cbuf;
  cbuf = new uint8_t[MAX_BUF_SIZE];

  transferActive = true;
  uint16_t bytesToSend = sprintf(cbuf,"AV+CTRANS,%u,%u;",_ift.uSize,_ift.uPackets);

  
  if(_nd->sendPayloadAndWait(cbuf,bytesToSend) == 0){
	 
	// Sending image via serial
	uint16_t responseTime = millis();
	bool sendAgain = false;
	
  
	while ((_ift.uPacketIndex < _ift.uPackets)){

    if((millis() - responseTime)>TRANSFER_TIMEOUT){
      Serial.println(F("Timeout"));
      break;

    }
    else if(!imgFile){
      Serial.println(F("IMG File problems"));

      break;
    }

		
		if(!sendAgain){
			bytesToSend = (_ift.uPacketIndex == _ift.uPackets-1)?(_ift.uSize - (_ift.uPacketIndex*MAX_BUF_SIZE)):MAX_BUF_SIZE;
			
			imgFile.read(cbuf, bytesToSend);
			
		}

		if(_nd->sendPayloadAndWait(cbuf,bytesToSend) == 0){
			sendAgain = false;
			responseTime = millis();
			_ift.uPacketIndex++;
		  
		}
		else{
			sendAgain = true;
			Serial.println(F("AV+ERR,XBEE,sendSnapshotFile"));
		}

	}
	
	  
  }
  else{
	  Serial.println(F("AV+ERR,XBEE,sendSnapshotFile"));
  }
  

  if((_ift.uPacketIndex < _ift.uPackets)) {
	  _nd->sendPayload(F("AV+ERR,TIMEOUT"));
    Serial.print(F("PacketIndex: "));
    Serial.print(_ift.uPacketIndex);
    Serial.print(F(" Packets: "));
    Serial.println(_ift.uPackets);
  }
  
  delete [] cbuf;
  imgFile.close();
  transferActive = false;
  _ift.uPacketIndex = 0;
  _ift.uPackets = 0;

}


