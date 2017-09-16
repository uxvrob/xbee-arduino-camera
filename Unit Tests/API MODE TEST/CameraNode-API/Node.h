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
#define MAX_BUF_SIZE 32         	   // Byte batch of image to send via Serial

#define XBEE_TIMEOUT 500

#define lowByte(w)           ((w) & 0xFF)
#define highByte(w)         (((w) >> 8) & 0xFF)


class Node {
	
	public:

    Stream* _s;

    //XBee* _xbee;
    XBeeWithCallbacks* _xbee; 
	
	XBeeResponse response;
	ZBRxResponse zbRx;
	ZBTxRequest  zbTx;
	ZBTxStatusResponse zbTxStatus;

    static const uint8_t ku8XBSuccess                = 0x00;
    static const uint8_t ku8XBIllegalDataAddress     = 0x02;
    static const uint8_t ku8XBResponseTimedOut       = 0xE2;
    static const uint8_t ku8XBDeliveryError          = 0x03;
    static const uint8_t ku8XBPacketError            = 0x04;
	  
	Node();
	  
	void begin();
    void beginCallbacks();
   
    void setPrintSerial(Stream*);
    void setXbeeSerial(Stream&);
    void setRxAddress(uint32_t, uint32_t);
    
	void setPayload(String);
	
	template <typename T>
	void setPayload(T* cbuf, uint8_t length){
		clearTransmitBuffer();
		for(uint8_t i =0; i<length; i++)
			setTransmitBuffer(i,(uint8_t)cbuf[i]);
		_u8TransmitBufferLength = length;
	}
	
	void sendPayload();
	uint8_t sendPayloadAndWait();
    void printXBAddress(void);
    void setReceiveCb(void(*)());

    uint8_t setTransmitBuffer(uint8_t, uint8_t);
    void clearTransmitBuffer();

    void spin();
    
    void debugOn();
    void debugOff();

    void printDirectory(File, int);
    bool getRecentImageFilename(char*);
    bool generateImageFilename(char*); 
	
	uint16_t convertFileSizeToPackets(uint16_t);
	uint16_t convertPacketToFilePosition(uint16_t, uint16_t);

    uint8_t _u8TransmitBuffer[MAX_BUF_SIZE];
    uint8_t _u8TransmitBufferLength;
    
	protected:
  
    bool _debugOn;

	private:

	XBeeAddress64 _addr64;
	void (*_zbReceiveCb)(ZBRxResponse&, uintptr_t);
	

	
};



#endif
