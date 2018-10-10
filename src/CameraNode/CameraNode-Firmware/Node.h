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
* Node.h
*
* @author Robbie Sharma <robbie@rsconsulting.ca>
* @date September 16, 2017
* @desc Implement to take images, save to SD, and transmit via xBee-Serial
 bridge.  Later, API commands will be used.
* 
***************************************************************/

#ifndef NODE_H
#define NODE_H

#include <XBee.h>
#include <Printers.h>

#include <SD.h>
#include <SPI.h>
#include <SoftwareSerial.h>      

#define SD_CHIP_SELECT_PIN 4        // SD card chip select
#define SER_XBEE_BAUD_RATE 57600 
#define MAX_BUF_SIZE 32         	   // Byte batch of image to send via Serial

#define XBEE_TIMEOUT 500
#define TRANSFER_TIMEOUT 5000

#define lowByte(w)           ((w) & 0xFF)
#define highByte(w)         (((w) >> 8) & 0xFF)


class Node {
	
	public:

    XBeeWithCallbacks* _xbee; 
	
	//XBeeResponse response;
	//ZBRxResponse zbRx;
	ZBTxRequest  zbTx;
	//ZBTxStatusResponse zbTxStatus;
	
	uint8_t _u8TransmitBuffer[MAX_BUF_SIZE];
    uint8_t _u8TransmitBufferLength;

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
    
	void sendPayload(String);
	
	template <typename T>
	void sendPayload(T* cbuf, uint8_t length){
		clearTransmitBuffer();
		for(uint8_t i =0; i<length; i++)
			setTransmitBuffer(i,(uint8_t)cbuf[i]);
		_u8TransmitBufferLength = length;
		
		_sendPayload();
	}
	
	template <typename T>
	uint8_t sendPayloadAndWait(T* cbuf, uint8_t length){
		
		clearTransmitBuffer();
		
		for(uint8_t i =0; i<length; i++)
			setTransmitBuffer(i,(uint8_t)cbuf[i]);
		_u8TransmitBufferLength = length;
		
		zbTx.setPayload(_u8TransmitBuffer,_u8TransmitBufferLength);
	
		return _xbee->sendAndWait(zbTx, XBEE_TIMEOUT);
	}
	
    void setReceiveCb(void(*)());

    uint8_t setTransmitBuffer(uint8_t, uint8_t);
    void clearTransmitBuffer();

    void spin();

    //void printDirectory(File, int);
    bool getRecentImageFilename(char*);
    bool generateImageFilename(char*); 
	
	void freeRam();
	
	uint16_t convertFileSizeToPackets(uint16_t);
	uint16_t convertPacketToFilePosition(uint16_t, uint16_t);


	private:

	XBeeAddress64 _addr64;

	void _sendPayload();
	
	void (*_zbReceiveCb)(ZBRxResponse&, uintptr_t);
	

	
};



#endif
