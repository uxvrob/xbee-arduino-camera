/*------------------------------------------------------------------------
  This file is part of the Smartifi - Avalon project

 <INSERT LICENSE SCHEME>
 
 
  ------------------------------------------------------------------------*/

#include "Node.h"


Node::Node()
{
	
}

void Node::begin(){

  if(SD_CHIP_SELECT_PIN != 10) pinMode(10, OUTPUT);

}

void Node::setSerial(Stream* s){

  _s = s;
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

