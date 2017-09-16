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
	
	  _nd->_s->print("FREE RAM: ");
  _nd->_s->println(_nd->freeRam());
	
    _nd->begin();
  
    // Try to locate the camera
  if (_cam->begin()) {
    _nd->_s->println(F("AV+CS,0x01;"));
	_nd->sendPayload("AV+CS,0x01;");
  } else {
    _nd->_s->println(F("AV+CS,0x00;"));
	_nd->sendPayload("AV+CS,0x00;");
    return;
  }

  // Set the picture size - you can choose one of 640x480, 320x240 or 160x120 
  // Remember that bigger pictures take longer to transmit!
  
  _cam->setImageSize(VC0706_640x480);        // biggest
  //_cam->setImageSize(VC0706_160x120);          // small
  
  //_cam->takePicture();
  
  //_cam->resumeVideo();
  
  _nd->_s->print(F("FREE RAM: "));
  _nd->_s->println(_nd->freeRam());

}

void CameraNode::takeSnapshotSaveToSD(){

  // Get snapshot
  if (!_cam->takePicture()) {
    _nd->_s->println(F("AV+ERR,TAKE_PIC;"));
    return;
  }

  // Create an image with incremental name IMAGExx.JPG
  
  /*
  if(!generateImageFilename(_ift.szName)){
    _nd->_s->println(F("AV+ERR,GEN_FILENAME;"));
    return;
  }
  */
  
  
  strcpy(_ift.szName, "IMAGE09.JPG");
  _ift.uSize = _cam->frameLength(); // Get the size of the image (frame) taken
  _ift.uPackets = _nd->convertFileSizeToPackets(_ift.uSize);
  _ift.uPacketIndex=0;

  
  if(!_debugOn){ 
	_nd->_s->print(F("AV+DEBUG,TAKESNAP_SD,"));
	_nd->_s->print(_ift.szName);
	_nd->_s->print(F(","));
	_nd->_s->print(_ift.uSize);
	_nd->_s->print(F(","));
	_nd->_s->print(_ift.uPackets);
	_nd->_s->print(F(","));
	_nd->_s->print(_ift.uPacketIndex);
	_nd->_s->println(F(";"));
  }
  
  
  File imgFile = SD.open(_ift.szName, FILE_WRITE);  // file object for image storage

  // Sending image via serial
  
  
  while (_ift.uPacketIndex < _ift.uPackets){
	
    // read IMG_MAX_BUF_SIZE bytes at a time;
	
    uint16_t bytesToRead = (_ift.uPacketIndex == _ift.uPackets-1)?(_ift.uSize - (_ift.uPacketIndex*MAX_BUF_SIZE)):MAX_BUF_SIZE;
 
	uint8_t* buf = _cam->readPicture(bytesToRead);
	
    imgFile.write(buf, bytesToRead);
	//_nd->_s->write(buf);
	//_nd->_s->print(F("FREE RAM: "));
	//_nd->_s->println(_nd->freeRam());
  
	_ift.uPacketIndex++;

  }
  
  _nd->_s->print(F("SUCCESS: "));
  _nd->_s->println(_ift.uSize);
  _nd->_s->print(F("FREE RAM: "));
  _nd->_s->println(_nd->freeRam());
  
  imgFile.flush();
  imgFile.close();

  

  if(!_cam->resumeVideo()){
    _nd->_s->println(F("AV+ERR,RESUME_VIDEO;"));
  }

}


void CameraNode::sendSnapshotFile(char* filename){

  if(!SD.exists(filename)){
    _nd->_s->println(F("AV+ERR,FILE_NO_EXIST;"));
    return;
  }
  
  File imgFile = SD.open(filename, FILE_READ);  // file object for image storage
  
  strcpy(_ift.szName,filename);
  _ift.uSize = imgFile.size();
  _ift.uPackets = _nd->convertFileSizeToPackets(_ift.uSize);
  _ift.uPacketIndex = 0;
  
  if(_ift.uSize == 0){
    _nd->_s->println(F("AV+ERR,FILE_SIZE_NULL;"));
    imgFile.rewindDirectory();
    imgFile.close();
    return;
  }
  
  
 
  char cbuf[MAX_BUF_SIZE];

  uint8_t bytesToSend = sprintf(cbuf,"AV+CTRANS,%d,%d;",_ift.uSize,_ift.uPackets);
  
  _nd->sendPayload(cbuf,bytesToSend);

  if(_nd->sendPayloadAndWait(cbuf,bytesToSend) == 0){
	  
	// Sending image via serial
	uint16_t responseTime = millis();
	bool sendAgain = false;
	
  
	while ((_ift.uPacketIndex < _ift.uPackets) && (millis() - responseTime)<10000){
    
		// read IMG_MAX_BUF_SIZE bytes at a time;
		
		
		if(!sendAgain){
			bytesToSend = (_ift.uPacketIndex == _ift.uPackets-1)?(_ift.uSize - (_ift.uPacketIndex*MAX_BUF_SIZE)):MAX_BUF_SIZE;
			
			imgFile.read(_nd->_u8TransmitBuffer, bytesToSend);
		}

		if(_nd->sendPayloadAndWait(_nd->_u8TransmitBuffer,bytesToSend) == 0){
			sendAgain = false;
			responseTime = millis();
			_ift.uPacketIndex++;
		  
		}
		else{
			sendAgain = true;
			_nd->_s->print(F("AV+ERR,XBEE,sendSnapshotFile"));
			//_nd->_s->println(_nd->_xbee.getResponse().getErrorCode());
		}

	}
	  
  }
  else{
	  _nd->_s->print(F("AV+ERR,XBEE,sendSnapshotFile"));
	  //_nd->_s->println(_nd->_xbee.getResponse().getErrorCode());
  }
  

  if((_ift.uPacketIndex < _ift.uPackets) && _debugOn) 
	  _nd->_s->println(F("AV+ERR,TIMEOUT"));
  
  imgFile.rewindDirectory();
  imgFile.close();
  

}


