/*------------------------------------------------------------------------
  This file is part of the Smartifi - Avalon project

 <INSERT LICENSE SCHEME>
 
 
  ------------------------------------------------------------------------*/

#include "Node.h"
#include "CameraNode.h"


CameraNode::CameraNode(int cam_rx_pin, int cam_tx_pin):
  _nd(this),
  _camConn(new SoftwareSerial(cam_rx_pin, cam_tx_pin)),
  _cam(new Adafruit_VC0706(this->_camConn))
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
  

  //if(SD.exists(_ift.szName)) SD.remove(_ift.szName);
  
  File imgFile = SD.open(_ift.szName, FILE_WRITE);  // file object for image storage

  
  //if(imgFile.seek(0)) Serial.println(F("Seek successful"));
  //if(imgFile.write("TEST") != 0) Serial.println(F("write success"));
  
  _nd->freeRam();
  
  if(!imgFile) Serial.println(F("AV+ERR,FILE_OPEN;"));
  
  while (_ift.uPacketIndex < _ift.uPackets && imgFile){
	
    // read IMG_MAX_BUF_SIZE bytes at a time;
	
    uint16_t bytesToRead = (_ift.uPacketIndex == _ift.uPackets-1)?(_ift.uSize - (_ift.uPacketIndex*MAX_BUF_SIZE)):MAX_BUF_SIZE;
 
	uint8_t* buf = _cam->readPicture(bytesToRead);
	imgFile.write(buf, bytesToRead);
	_ift.uPacketIndex++;

  }
  
  
  imgFile.flush();
  imgFile.close();  

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
  
  uint8_t cbuf[MAX_BUF_SIZE];

  uint16_t bytesToSend = sprintf(cbuf,"AV+CTRANS,%u,%u;",_ift.uSize,_ift.uPackets);

  
  if(_nd->sendPayloadAndWait(cbuf,bytesToSend) == 0){
	 
	// Sending image via serial
	uint16_t responseTime = millis();
	bool sendAgain = false;
	
  
	while ((_ift.uPacketIndex < _ift.uPackets) && (millis() - responseTime)<1000){

		
		if(!sendAgain){
			bytesToSend = (_ift.uPacketIndex == _ift.uPackets-1)?(_ift.uSize - (_ift.uPacketIndex*MAX_BUF_SIZE)):MAX_BUF_SIZE;
			
			imgFile.read(cbuf, bytesToSend);
			
			Serial.print(F("Packet: "));
			Serial.println(_ift.uPacketIndex);
		}

		if(_nd->sendPayloadAndWait(cbuf,bytesToSend) == 0){
			sendAgain = false;
			responseTime = millis();
			
			_ift.uPacketIndex++;
		  
		}
		else{
			sendAgain = true;
			Serial.print(F("AV+ERR,XBEE,sendSnapshotFile"));
			//Serial.println(_nd->_xbee.getResponse().getErrorCode());
		}

	}
	
	  
  }
  else{
	  Serial.print(F("AV+ERR,XBEE,sendSnapshotFile"));
	  //Serial.println(_nd->_xbee.getResponse().getErrorCode());
  }
  

  if((_ift.uPacketIndex < _ift.uPackets)) 
	  Serial.println(F("AV+ERR,TIMEOUT"));
  
  imgFile.close();
  

}


