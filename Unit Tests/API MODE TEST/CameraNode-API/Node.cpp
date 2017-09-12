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
_s(&Serial),
_xbee(new XBeeWithCallbacks())
{
	
}

void Node::setPrintSerial(Stream* s){
  _s = s;
}



void Node::begin(){

  // see if the card is present and can be initialized:
  if(SD_CHIP_SELECT_PIN != 10) pinMode(10, OUTPUT);
  if (!SD.begin(SD_CHIP_SELECT_PIN)) _s->println(F("AV+ERR,0x01;"));

  this->beginCallbacks();
  
}

void Node::spin(){

  _xbee->loop();
}

void Node::debugOn(){
  _debugOn = true;
}

void Node::debugOff(){
  _debugOn = false;
}

void Node::setXbeeSerial(Stream& ser){

  _xbee->setSerial(ser);
  /*
  if(!ser)
    ser.begin(SER_XBEE_BAUD_RATE);
  */
}

void Node::setRxAddress(uint32_t msb, uint32_t lsb){
  _msb = msb;
  _lsb = lsb;
  
}
void Node::printXBAddress(){

  _s->print("MSB: ");
  _s->print(_msb,HEX);
  _s->print(" LSB: ");
  _s->println(_lsb,HEX);
}

void Node::beginCallbacks(){
  
  _xbee->onPacketError(printErrorCb, (uintptr_t)(Print*)&_s);
  _xbee->onTxStatusResponse(printErrorCb, (uintptr_t)(Print*)&_s);
  _xbee->onZBTxStatusResponse(printErrorCb, (uintptr_t)(Print*)&_s);

  _xbee->onZBRxResponse(this->_zbReceiveCb);
  //_xbee->onRx16Response(receive16);
  //_xbee->onRx64Response(receive64);
  
}

void Node::setReceiveCb(void (*zbReceiveCb)()){
  _zbReceiveCb = zbReceiveCb;
}

uint8_t Node::sendPayload(){

  uint8_t result = ku8XBSuccess;

  XBeeAddress64 addr64 = XBeeAddress64(_msb, _lsb);
  ZBTxRequest zbTx = ZBTxRequest(addr64, _u8TransmitBuffer, _u8TransmitBufferLength);
  ZBTxStatusResponse txStatus = ZBTxStatusResponse();
  
  _xbee->send(zbTx);
  
  if(_xbee->readPacket(500)){
      if (_xbee->getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
        
         _xbee->getResponse().getZBTxStatusResponse(txStatus);

      // get the delivery status, the fifth byte
      if (txStatus.getDeliveryStatus() == SUCCESS) {

        result = ku8XBSuccess;

      } else {
        // the remote XBee did not receive our packet. is it powered on?
        result = ku8XBDeliveryError;
      }
    }
    
  }else if (_xbee->getResponse().isError()){
    //_s->print("Error reading packet.  Error code: ");  
    //_s->println(_xbee->getResponse().getErrorCode());
    result = ku8XBPacketError;
  }
  else{
    result = ku8XBResponseTimedOut;
  }
 
  
  return result;  
  
}

/**
Retrieve data from response buffer.

@see Node::clearResponseBuffer()
@param u8Index index of response buffer array (0x00..0x3F)
@return value in position u8Index of response buffer (0x0000..0xFFFF)
@ingroup buffer
*/
uint8_t Node::getResponseBuffer(uint8_t u8Index) {
  if (u8Index < ku8MaxBufferSize) {
    return _u8ResponseBuffer[u8Index];
  } else {
    return 0xFF;
  }
}

/**
Clear Node response buffer.

@see Node::getResponseBuffer(uint8_t u8Index)
@ingroup buffer
*/
void Node::clearResponseBuffer() {
  uint8_t i;

  for (i = 0; i < ku8MaxBufferSize; i++) {
    _u8ResponseBuffer[i] = 0;
  }
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
  if (u8Index < ku8MaxBufferSize) {
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

  for (i = 0; i < ku8MaxBufferSize; i++) {
    _u8TransmitBuffer[i] = 0;
  }
}

void Node::printDirectory(File dir, int numTabs) {
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      _s->print('\t');
    }
    _s->print(entry.name());
    if (entry.isDirectory()) {
      _s->println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      _s->print("\t\t");
      _s->println(entry.size(), DEC);
    }
    entry.close();
  }
}


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
  
  return true;
}


