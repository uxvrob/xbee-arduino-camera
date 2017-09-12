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

#define SD_CHIP_SELECT_PIN 4        // SD card chip select


class Node {
	
	public:

    Stream* _s;
	  
	  Node();
	  
	  void begin();
    void setSerial(Stream*);

    

    void debugOn();
    void debugOff();

    void printDirectory(File, int);
    bool getRecentImageFilename(char*);
    bool generateImageFilename(char*); 
	
	protected:
  
    bool _debugOn;
    
	
	
};



#endif
