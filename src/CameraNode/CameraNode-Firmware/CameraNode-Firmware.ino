/***************************************************************
* Camera Node firmware
*
* @author Robbie Sharma robbie -at- rsconsulting.ca
* @date August 21, 2017
* @desc Before upload, flip switch to "USB" on the arduino Shield
* 
***************************************************************/

#include <Adafruit_VC0706.h>
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>      
#include <Time.h>  

#define ND_CMD_BUF_SIZE 200     // Command Buffer maximum size
#define IMG_BUF_SIZE 64         // Byte batch of image to send via Serial
#define SD_CHIP_SELECT_PIN 4    // SD card chip select
#define CAM_RX_PIN 2
#define CAM_TX_PIN 3
#define SER_BAUD_RATE 57600    // Serial baud rate
#define TRANSMISSION_DELAY 100 // in microseconds (us)

/* _____UTILITY MACROS_______________________________________________________ */
/**
@def lowWord(ww) ((uint16_t) ((ww) & 0xFFFF))
Macro to return low word of a 32-bit integer.
*/
#define lowWord(ww) ((uint16_t) ((ww) & 0xFFFF))


/**
@def highWord(ww) ((uint16_t) ((ww) >> 16))
Macro to return high word of a 32-bit integer.
*/
#define highWord(ww) ((uint16_t) ((ww) >> 16))


/**
@def LONG(hi, lo) ((uint32_t) ((hi) << 16 | (lo)))
Macro to generate 32-bit integer from (2) 16-bit words.
*/
#define LONG(hi, lo) ((uint32_t) ((hi) << 16 | (lo)))





/* _____PROJECT INCLUDES_____________________________________________________ */
// functions to calculate Modbus Application Data Unit CRC

#define lowByte(w)           ((w) & 0xFF)
#define highByte(w)         (((w) >> 8) & 0xFF)


boolean debugOn = false;

typedef struct {
  char szName[25];
  uint16_t uSize;
  uint16_t uCRC32;
} image_file_t;

// Command buffer initialization
String cmdBuf;
boolean cmdComplete = false;

// Serial and camera connections
SoftwareSerial cameraconnection = SoftwareSerial(CAM_RX_PIN,CAM_TX_PIN);
Adafruit_VC0706 cam = Adafruit_VC0706(&cameraconnection);



uint16_t word(uint8_t high, uint8_t low) {
  uint16_t ret_val = low;
  ret_val += (high << 8);
  return ret_val;
}

uint16_t _crc16_update(uint16_t crc, uint8_t a) {
  int i;

  crc ^= a;
  for (i = 0; i < 8; ++i)
  {
    if (crc & 1)
      crc = (crc >> 1) ^ 0xA001;
    else
      crc = (crc >> 1);
  }
  return crc;
}

// Get image size from the camera 
void getImageSize(){
  
  uint8_t imgsize = cam.getImageSize();
  
  Serial.print(F("AV+IMGS,"));
  if (imgsize == VC0706_640x480) Serial.println(VC0706_640x480);
  if (imgsize == VC0706_320x240) Serial.println(VC0706_320x240);
  if (imgsize == VC0706_160x120) Serial.println(VC0706_160x120);
  
}

void getCameraVersion(){
  
  // Print out the camera version information (optional)  
  char *reply = cam.getVersion();
  
  if (reply == 0) {
    Serial.println(F("AV+ERR,0x0000"));
  } else {
    Serial.print(F("AV+CSV,"));
    Serial.println(reply);
  }
  
}

void printDirectory(File dir, int numTabs) {
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}

bool generateImageFilename(char* szFileName){
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

bool getRecentImageFilename(char* szFileName){
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

void takeSnapshotSaveToSD(image_file_t* ift){

  // Get snapshot
  if (!cam.takePicture()) {
    Serial.println(F("AV+ERR,TAKE_PIC;"));
    return;
  }

  uint8_t *buffer;                     // transmission buffer
  uint8_t bytesToRead;                 // bytes in transmission buffer
  uint16_t jpglen = cam.frameLength(); // Get the size of the image (frame) taken
  File imgFile;                        // file object for image storage

  // Create an image with incremental name IMAGExx.JPG
  char filename[13];

  if(!generateImageFilename(filename)){
    Serial.println("AV+ERR,GEN_FILENAME;");
    return;
  }
  
  // set ift params
  strcpy(ift->szName, filename);
  ift->uSize = jpglen;


  if(debugOn) Serial.print("AV+DEBUG,TAKESNAP_SD,"+String(ift->szName)+","+String(ift->uSize)+";");
  
  
  // Open the file for writing
  imgFile = SD.open(filename, FILE_WRITE);

  // Sending image via serial
  
  while (jpglen > 0){
    
    // read IMG_BUF_SIZE bytes at a time;
    uint8_t bytesToRead = min(IMG_BUF_SIZE, jpglen); //change 32 to 64 for a speedup but may not work with all setups!
 
    buffer = cam.readPicture(bytesToRead);

    imgFile.write(buffer, bytesToRead);
    jpglen -= bytesToRead;
    
  }

  // Determine CRC of file
  ift->uCRC32 = 0;

  imgFile.flush();
  imgFile.close();

  if(!cam.resumeVideo()){
    Serial.println("AV+ERR,RESUME_VIDEO;");
    return false;
  }

  return true;
}

void takeSnapshotTransmitSaveToSD(image_file_t* ift){

  // Get snapshot
  if (!cam.takePicture()) {
    Serial.println("AV+ERR,TAKE_PIC;");
    return;
  }

  uint8_t *buffer;                     // transmission buffer
  uint8_t bytesToRead;                 // bytes in transmission buffer
  uint16_t jpglen = cam.frameLength(); // Get the size of the image (frame) taken
  File imgFile;                        // file object for image storage

  // Create an image with incremental name IMAGExx.JPG
  char filename[13];

  if(!generateImageFilename(filename)){
    Serial.println("AV+ERR,GEN_FILENAME;");
    return;
  }
  
  // set ift params
  strcpy(ift->szName, filename);
  ift->uSize = jpglen;

  if(debugOn) Serial.print("AV+DEBUG,TAKESNAP_SD,"+String(ift->szName)+","+String(ift->uSize)+";");

  //Send transmission packet with img file length
  Serial.print("AV+CTRANS,");
  Serial.print(jpglen);
  Serial.write(";");
  
  
  // Open the file for writing
  imgFile = SD.open(filename, FILE_WRITE);

  // Sending image via serial
  
  while (jpglen > 0){
    
    // read IMG_BUF_SIZE bytes at a time;
    uint8_t bytesToRead = min(IMG_BUF_SIZE, jpglen); //change 32 to 64 for a speedup but may not work with all setups!
 
    buffer = cam.readPicture(bytesToRead);

    imgFile.write(buffer, bytesToRead);
    Serial.write(buffer, bytesToRead);
    
    jpglen -= bytesToRead;
    
  }

  imgFile.flush();
  imgFile.close();

  if(!cam.resumeVideo()){
    Serial.println(F("AV+ERR,RESUME_VIDEO;"));
    return false;
  }

  return true;
}

void sendBuffer(uint8_t* buffer, uint8_t bytesToRead){
  uint16_t u16CRC = 0xFFFF;

  uint16_t o = 0;
  
  o+= Serial.write(0x76);
  o+= Serial.write(bytesToRead);

  u16CRC = 0xFFFF;
  
  for (uint8_t i = 0; i < bytesToRead; i++) {
    u16CRC = _crc16_update(u16CRC, buffer[i]);
  }


  if(debugOn){
      Serial.print("CRC LOW :");
      Serial.print(lowByte(u16CRC),HEX);
      Serial.print(" HIGH: ");
      Serial.println(highByte(u16CRC),HEX);
  
  }
  o+= Serial.write(buffer, bytesToRead);

  o+= Serial.write(bytesToRead);
  o+= Serial.write(bytesToRead);
  /*
  o+= Serial.write(lowByte(u16CRC));
  o+= Serial.write(highByte(u16CRC));
  */
  o+= Serial.write(0);
  
  if(debugOn) { 
     Serial.print("Bytes written: ");
     Serial.println(o,HEX);
  }
  
  return;
        
}

void sendSnapshotFile(char* filename){

  if(!SD.exists(filename)){
    Serial.println("AV+ERR,FILE_NO_EXIST;");
    return;
  }
  
  File imgFile = SD.open(filename, FILE_READ);  // file object for image storage
  uint8_t buffer[IMG_BUF_SIZE];                              // transmission buffer
  uint8_t bytesToRead;                          // bytes in transmission buffer
  uint16_t jpglen = imgFile.size();          // Get the size of the image (frame) taken
  uint8_t sendAttempts = 0;
  

  //Send transmission packet with img file length
  Serial.print(F("AV+CTRANS,"));
  Serial.print(jpglen);
  Serial.write(";");

  // Sending image via serial
  uint16_t responseTime = millis();
  while (jpglen > 0 && (millis() - responseTime)<10000){
    
    // read IMG_BUF_SIZE bytes at a time;
    if(cmdComplete){

      cmdBuf.trim();

      if(cmdBuf.equals("+")){
        
        bytesToRead = min(IMG_BUF_SIZE, jpglen);

        imgFile.read(buffer, bytesToRead);
  
        sendBuffer(buffer, bytesToRead);
        
        jpglen -= bytesToRead;        
        sendAttempts = 0;
        
      }else if(cmdBuf.equals("-")){
        sendBuffer(buffer, bytesToRead);
      }
      else {
        if(sendAttempts > 5)
          break;
        if(debugOn){
          Serial.print(F("INVALID CMD. Attempt "));
          Serial.print(sendAttempts,HEX);
          Serial.println(F(" of 5"));
        }
        sendAttempts++;
        delay(100);        
      }

      responseTime = millis();
      cmdBuf = "";
      cmdComplete = false;
    }
    processSerial();
  }

  if(jpglen >0 && debugOn) Serial.println("Transfer issues...");
  
  imgFile.close();
  

}


void setup() {
  
  if(SD_CHIP_SELECT_PIN != 10) pinMode(10, OUTPUT); // SS on Uno, etc.

  Serial.begin(SER_BAUD_RATE);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(SD_CHIP_SELECT_PIN)) {
    Serial.println(F("AV+ERR,0x01;"));
  }  
  
  // Try to locate the camera
  if (cam.begin()) {
    Serial.println(F("AV+CS,0x01;"));
  } else {
    Serial.println(F("AV+CS,0x00;"));
    return;
  }

  // Set the picture size - you can choose one of 640x480, 320x240 or 160x120 
  // Remember that bigger pictures take longer to transmit!
  
  cam.setImageSize(VC0706_640x480);        // biggest
  //cam.setImageSize(VC0706_160x120);          // small

  cmdBuf.reserve(ND_CMD_BUF_SIZE);
  
}

void loop(){

    // Command state machine
    // If a command string terminated by \n is received, process the string

    image_file_t* ift;
    ift = new image_file_t[1];
    
    
    if(cmdComplete){

      cmdBuf.trim();
      
      if(cmdBuf.equals("AV+CGETS")){        // Send snapshot via Serial routine

        takeSnapshotTransmitSaveToSD(ift);


      }else if(cmdBuf.equals("AV+SGETS")){        // Send snapshot via Serial routine
        
        takeSnapshotSaveToSD(ift);
        sendSnapshotFile(ift->szName);

      }else if(cmdBuf.equals("AV+SNAP")){
        
        takeSnapshotSaveToSD(ift);
        
      }else if(cmdBuf.equals("AV+RECF")){
        if(getRecentImageFilename(ift->szName)){
          Serial.print(F("Recent image: "));
          Serial.println(ift->szName);
        }
        else
          Serial.println(F("No file created yet"));
        
      }else if(cmdBuf.equals("AV+FILES")){
        
        File root = SD.open("/");
        root.rewindDirectory();
        printDirectory(root, 0);
        root.rewindDirectory();
        root.close();
      
      }else if(cmdBuf.equals("AV+CSEND")){
        
        getRecentImageFilename(ift->szName);
        sendSnapshotFile(ift->szName);
      
      }else if(cmdBuf.equals("AV+DEBUGON")){

        debugOn = true;
      
      }else if(cmdBuf.equals("AV+JRES")){   // Command to reset the arduino.  NOT IMPLEMENTED YET.
      
        Serial.println(F("ROK"));
      
      }else if(cmdBuf.equals("OK")){        // ACK type command
        
          
      }else{                                // Invalid CMD received
        //Serial.print("NOK: ");
        //Serial.println(cmdBuf);
        //Serial.write('\n');
      }

      // Reset command buffers
      cmdBuf = "";
      cmdComplete = false;
  }

  processSerial();

  delete [] ift;
}

// Thread for processing serial events.  A byte or char is added to a buffer until 
// a newline character is received.

void processSerial(){

  int bytesAvail = Serial.available();

  if(bytesAvail > 0){
  
    for(int i = 0; i < bytesAvail; i++){
      
      char inChar = (char)Serial.read();
      cmdBuf += inChar;
  
      if(inChar == '\n'){
        cmdComplete = true;
      }
      
    }
  }
  
}
