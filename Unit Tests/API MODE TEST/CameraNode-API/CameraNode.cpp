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
    _nd->_s->println(F("AV+CS,0x01;"));
  } else {
    _nd->_s->println(F("AV+CS,0x00;"));
    return;
  }

  // Set the picture size - you can choose one of 640x480, 320x240 or 160x120 
  // Remember that bigger pictures take longer to transmit!
  
  _cam->setImageSize(VC0706_640x480);        // biggest
  //_cam->setImageSize(VC0706_160x120);          // small

}

void CameraNode::takeSnapshotSaveToSD(image_file_t* ift){

  // Get snapshot
  if (!_cam->takePicture()) {
    _nd->_s->println(F("AV+ERR,TAKE_PIC;"));
    return;
  }

  uint8_t *buffer;                     // transmission buffer
  uint8_t bytesToRead;                 // bytes in transmission buffer
  uint16_t jpglen = _cam->frameLength(); // Get the size of the image (frame) taken
  File imgFile;                        // file object for image storage

  // Create an image with incremental name IMAGExx.JPG
  char filename[13];

  if(!generateImageFilename(filename)){
    _nd->_s->println("AV+ERR,GEN_FILENAME;");
    return;
  }
  
  // set ift params
  strcpy(ift->szName, filename);
  ift->uSize = jpglen;


  if(Node::_debugOn) _nd->_s->print("AV+DEBUG,TAKESNAP_SD,"+String(ift->szName)+","+String(ift->uSize)+";");
  
  
  // Open the file for writing
  imgFile = SD.open(filename, FILE_WRITE);

  // Sending image via serial
  
  while (jpglen > 0){
    
    // read IMG_BUF_SIZE bytes at a time;
    uint8_t bytesToRead = min(BUF_SIZE, jpglen); //change 32 to 64 for a speedup but may not work with all setups!
 
    buffer = _cam->readPicture(bytesToRead);

    imgFile.write(buffer, bytesToRead);
    jpglen -= bytesToRead;
    
  }

  imgFile.flush();
  imgFile.close();

  if(!_cam->resumeVideo()){
    _nd->_s->println("AV+ERR,RESUME_VIDEO;");
    return false;
  }

  return true;
}


void CameraNode::sendSnapshotFile(char* filename){

  if(!SD.exists(filename)){
    _nd->_s->println("AV+ERR,FILE_NO_EXIST;");
    return;
  }
  
  File imgFile = SD.open(filename, FILE_READ);  // file object for image storage
  
  uint8_t bytesToRead;                          // bytes in transmission buffer
  uint16_t jpglen = imgFile.size();          // Get the size of the image (frame) taken
  uint8_t sendAttempts = 0;
  char cbuf[BUF_SIZE];
  
  if(jpglen == 0){
    _nd->_s->println(F("AV+ERR,FILE_SIZE_NULL;"));
    imgFile.rewindDirectory();
    imgFile.close();
    return;
  }

  _nd->clearTransmitBuffer();

  uint8_t bytes = sprintf(cbuf,"AV+CTRANS,%d;",jpglen);

  for(uint8_t i =0; i<bytes; i++)
    _nd->setTransmitBuffer(i,(uint8_t)cbuf[i]);

  _nd->_u8TransmitBufferLength = bytes;

  if(!_nd->sendPayload())
    Serial.println("Did not get proper response");
  

  // Sending image via serial
  uint16_t responseTime = millis();
  
  while (jpglen > 0 && (millis() - responseTime)<10000){
    
    // read IMG_BUF_SIZE bytes at a time;
        
    bytesToRead = min(BUF_SIZE, jpglen);

    imgFile.read(_nd->_u8TransmitBuffer, bytesToRead);
    _nd->_u8TransmitBufferLength = bytesToRead;

    switch(_nd->sendPayload()){
      
      case ku8XBResponseTimedOut:
      case ku8XBDeliveryError:
      case ku8XBPacketError:
        imgFile.seek(imgFile.size()-jpglen);
        break;
      case ku8XBSuccess:
        responseTime = millis();
        jpglen -= bytesToRead;
        break;        
    
    }

  }

  if(jpglen >0 && _debugOn) _nd->_s->println(F("AV+ERR,TIMEOUT"));
  imgFile.rewindDirectory();
  imgFile.close();
  

}



