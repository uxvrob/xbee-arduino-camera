/*------------------------------------------------------------------------
  This file is part of the Smartifi - Avalon project

 <INSERT LICENSE SCHEME>
 
 
  ------------------------------------------------------------------------*/
#ifndef NODE_H
#define NODE_H

#include <XBee.h>
#include <Printers.h>

#include <SD.h>
#include <SPI.h>
#include <SoftwareSerial.h>      
#include <Time.h>

#define SD_CHIP_SELECT_PIN 4        // SD card chip select
#define SER_XBEE_BAUD_RATE 57600 


class Node {
	
	public:

    Stream* _s;

    XBee* _xbee;
    XBeeResponse* _response;
    ZBRxResponse* _rx;
    ZBTxStatusResponse* _txStatus;

    static const uint8_t ku8XBSuccess                = 0x00;
    static const uint8_t ku8XBResponseTimedOut       = 0xE2;
    static const uint8_t ku8XBDeliveryError          = 0x03;
    static const uint8_t ku8XBPacketError            = 0x04;
	  
	  Node();
	  
	  void begin();
   
    void setPrintSerial(Stream*);
    void setXbeeSerial(Stream&);
    bool setRxAddress(uint32_t, uint32_t);
    uint8_t sendPayload(uint8_t*, uint8_t);
    void printXBAddress(void);
    

    void debugOn();
    void debugOff();

    void printDirectory(File, int);
    bool getRecentImageFilename(char*);
    bool generateImageFilename(char*); 

    
	
	protected:
  
    bool _debugOn;

  private:

    XBeeAddress64* _addr64;

    
    
	
	
};



#endif
