/*------------------------------------------------------------------------
  This file is part of the Smartifi - Avalon project

 <INSERT LICENSE SCHEME>
 
 
  ------------------------------------------------------------------------*/

#include "Node.h"


uint16_t word(uint8_t high, uint8_t low) {
  uint16_t ret_val = low;
  ret_val += (high << 8);
  return ret_val;
}


Node::Node():
_xbee(new XBeeWithCallbacks())
{
	
}


void Node::begin(){

  // see if the card is present and can be initialized:
  if(SD_CHIP_SELECT_PIN != 10) pinMode(10, OUTPUT);
  if (!SD.begin(SD_CHIP_SELECT_PIN)) Serial.println(F("AV+ERR,0x01;"));

  this->beginCallbacks();
  
}

void Node::spin(){
  _xbee->loop();
}

void Node::setXbeeSerial(Stream& ser){

  _xbee->setSerial(ser);

}

void Node::setRxAddress(uint32_t msb, uint32_t lsb){
	_addr64.setMsb(msb);
	_addr64.setLsb(lsb);
	zbTx.setAddress64(_addr64);
}

void Node::beginCallbacks(){
  
  //_xbee->onPacketError(printErrorCb, (uintptr_t)(Print*)&_s);
  //_xbee->onTxStatusResponse(printErrorCb, (uintptr_t)(Print*)&_s);
  _xbee->onZBTxStatusResponse(printErrorCb, (uintptr_t)(Print*)&Serial);

  _xbee->onZBRxResponse(this->_zbReceiveCb);
  //_xbee->onRx16Response(receive16);
  //_xbee->onRx64Response(receive64);
  
}

void Node::setReceiveCb(void (*zbReceiveCb)()){
  _zbReceiveCb = zbReceiveCb;
}


void Node::sendPayload(String str){
	clearTransmitBuffer();
	for(uint8_t i =0; i<str.length(); i++)
		setTransmitBuffer(i,(uint8_t)str[i]);

	_u8TransmitBufferLength = str.length();
	
	_sendPayload();

	
}

void Node::_sendPayload(){

	zbTx.setPayload(_u8TransmitBuffer,_u8TransmitBufferLength);
	_xbee->send(zbTx);
	
}


/**
Place data in transmit buffer.

@see Node::clearTransmitBuffer()
@param u8Index index of transmit buffer array (0x00..0x3F)
@param u8Value value to place in position u8Index of transmit buffer (0x00..0xFF)
@return 0 on success; exception number on failure
@ingroup buffer
*/
uint8_t Node::setTransmitBuffer(uint8_t u8Index, uint8_t u8Value) {
  if (u8Index < MAX_BUF_SIZE) {
    _u8TransmitBuffer[u8Index] = u8Value;
    return ku8XBSuccess;
  } else {
    return ku8XBIllegalDataAddress;
  }
}


/**
Clear Node transmit buffer.

@see Node::setTransmitBuffer(uint8_t u8Index, uint16_t u16Value)
@ingroup buffer
*/
void Node::clearTransmitBuffer() {
  uint8_t i;

  for (i = 0; i < MAX_BUF_SIZE; i++) {
    _u8TransmitBuffer[i] = 0;
  }
}

/*
void Node::printDirectory(File dir, int numTabs) {
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print(F("\t"));

    }
    Serial.print(entry.name());
	


    if (entry.isDirectory()) {
      Serial.println(F("/"));
	  

	  
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print(F("\t\t"));
      Serial.println(entry.size(), DEC);

	
    }
    entry.close();
  }
}
*/

bool Node::generateImageFilename(char* szFileName){
  strcpy(szFileName, "IMAGE00.JPG");
  for (int i = 0; i < 1000; i++) {
    szFileName[5] = '0' + i/10;
    szFileName[6] = '0' + i%10;
    // create if does not exist, do not open existing, write, sync after write
    if (! SD.exists(szFileName)) {
      break;
    }
  }
  
  return true;
}

bool Node::getRecentImageFilename(char* szFileName){
  strcpy(szFileName, "IMAGE00.JPG");
  int maxVal;
  
  for (int i = 0; i < 1000; i++) {
    szFileName[5] = '0' + i/10;
    szFileName[6] = '0' + i%10;
    
    // create if does not exist, do not open existing, write, sync after write
    
    if (!SD.exists(szFileName)) {
      
      if(i == 0){
        strcpy(szFileName, "");
        return false;
      }
      maxVal = i-1;
      break;
    }
  }


  szFileName[5] = '0' + maxVal/10;
  szFileName[6] = '0' + maxVal%10;
  
  delay(100);
  
  return true;
}

uint16_t Node::convertFileSizeToPackets(uint16_t fileSize){
	
	uint16_t numPackets = static_cast<int>((fileSize/MAX_BUF_SIZE));
	numPackets += (fileSize % MAX_BUF_SIZE) == 0 ? 0 : 1;
	
	return numPackets;
}

uint16_t Node::convertPacketToFilePosition(uint16_t packetIndex, uint16_t fileSize){
	
	uint16_t pos = packetIndex*MAX_BUF_SIZE;
	
	if(pos > fileSize) return 1;
	else return pos;
	
}

void Node::freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  Serial.print(F("Free Ram: "));
  Serial.println( (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval)); 
}

