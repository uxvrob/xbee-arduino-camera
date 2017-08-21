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
#define SER_BAUD_RATE 115200    // Serial baud rate

// Command buffer initialization
String cmdBuf;
boolean cmdComplete = false;
// Serial and camera connections
SoftwareSerial cameraconnection = SoftwareSerial(CAM_RX_PIN,CAM_TX_PIN);
Adafruit_VC0706 cam = Adafruit_VC0706(&cameraconnection);

// Get image size from the camera 
void getImageSize(){
  
  uint8_t imgsize = cam.getImageSize();
  
  Serial.print("AV+IMGS,");
  if (imgsize == VC0706_640x480) Serial.println(VC0706_640x480);
  if (imgsize == VC0706_320x240) Serial.println(VC0706_320x240);
  if (imgsize == VC0706_160x120) Serial.println(VC0706_160x120);
  
}

void getCameraVersion(){
  
  // Print out the camera version information (optional)  
  char *reply = cam.getVersion();
  
  if (reply == 0) {
    Serial.println("AV+ERR,0x0000");
  } else {
    Serial.print("AV+CSV,");
    Serial.println(reply);
  }
  
}

void sendSnapshot(bool saveToSD = true){

  // Get snapshot
  if (!cam.takePicture()) {
    Serial.println("AV+ERR,0x02;");
    return;
  }

  uint8_t *buffer;                     // transmission buffer
  uint8_t bytesToRead;                 // bytes in transmission buffer
  uint16_t jpglen = cam.frameLength(); // Get the size of the image (frame) taken
  File imgFile;                        // file object for image storage

  // Create an image with incremental name IMAGExx.JPG
  if(saveToSD){

    char filename[13];
  
    strcpy(filename, "IMAGE00.JPG");
    for (int i = 0; i < 1000; i++) {
      filename[5] = '0' + i/10;
      filename[6] = '0' + i%10;
      // create if does not exist, do not open existing, write, sync after write
      if (! SD.exists(filename)) {
        break;
      }
    }
    
    // Open the file for writing
    imgFile = SD.open(filename, FILE_WRITE);
  }
  
  //Send transmission packet with img file length
  Serial.print("AV+CTRANS,");
  Serial.print(jpglen);
  Serial.write(";");

  // Sending image via serial
  
  while (jpglen > 0){
    
    // read IMG_BUF_SIZE bytes at a time;
    uint8_t bytesToRead = min(IMG_BUF_SIZE, jpglen); //change 32 to 64 for a speedup but may not work with all setups!
 
    buffer = cam.readPicture(bytesToRead);
    Serial.write(buffer,bytesToRead);

    if(saveToSD) imgFile.write(buffer, bytesToRead);
    jpglen -= bytesToRead;
    
  }
  //Serial.print("AV+CTRANF;"); 
  
  if(saveToSD){
    imgFile.close();
  }
  
}


void setup() {
  
  if(SD_CHIP_SELECT_PIN != 10) pinMode(10, OUTPUT); // SS on Uno, etc.

  Serial.begin(SER_BAUD_RATE);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(SD_CHIP_SELECT_PIN)) {
    Serial.println("AV+ERR,0x01;");
  }  
  
  // Try to locate the camera
  if (cam.begin()) {
    Serial.println("AV+CS,0x01;");
  } else {
    Serial.println("AV+CS,0x00;");
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
    
    if(cmdComplete){

      cmdBuf.trim();
      
      if(cmdBuf.equals("AV+CGETS")){        // Send snapshot via Serial routine
        sendSnapshot(false);
      }else if(cmdBuf.equals("AV+JRES")){   // Command to reset the arduino.  NOT IMPLEMENTED YET.
        Serial.println("ROK");
      }else if(cmdBuf.equals("OK")){        // ACK type command
        
          
      }else{                                // Invalid CMD received
        Serial.print("NOK: ");
        Serial.println(cmdBuf);
        Serial.write('\n');
      }

      // Reset command buffers
      cmdBuf = "";
      cmdComplete = false;
  }
}

// Thread for processing serial events.  A byte or char is added to a buffer until 
// a newline character is received.

void serialEvent(){
  
  while(Serial.available()){
    
    char inChar = (char)Serial.read();
    cmdBuf += inChar;

    if(inChar == '\n'){
      cmdComplete = true;
    }
    
  }
  
}
