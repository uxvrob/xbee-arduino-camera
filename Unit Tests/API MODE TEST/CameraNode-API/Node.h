/*------------------------------------------------------------------------
  This file is part of the Smartifi - Avalon project

 <INSERT LICENSE SCHEME>
 
 
  ------------------------------------------------------------------------*/
#ifndef NODE_H
#define NODE_H

#include <XBee.h>
#include <SD.h>
#include <SPI.h>
#include <SoftwareSerial.h>      
#include <Time.h>

#include <Adafruit_VC0706.h>

#define IMG_BUF_SIZE 64         // Byte batch of image to send via Serial
#define SD_CHIP_SELECT_PIN 4    // SD card chip select

class Node {
	
	public:
	  
	  Node();
	  
	  void begin();
    void setSerial(Stream*);
	  /*void send();
	  void send_packet();
	  void receive();
	  void receive_packet();
	  */

    void printDirectory(File, int);
    bool generateImageFilename(char*);
	
	private:

    Stream* _s;


	
	
	
};



#endif
