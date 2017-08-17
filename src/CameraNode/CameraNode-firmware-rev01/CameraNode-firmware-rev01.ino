/***************************************************************
* Camera Node firmware 
*
* Before upload, flip switch to "USB" on the arduino Shield
* After upload, flip switch to "micro"
***************************************************************/

#include <Adafruit_VC0706.h>
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>      
#include <Time.h>  
//#include "Timer.h"
//#include <XBee.h>
//#include <SimpleTimer.h>

#define node_id = 42134423
#define ND_CMD_BUF_SIZE 200

#define IMG_BUF_SIZE 32

// SD card chip select
#define chipSelect 4

// Camera pins
#define CAM_RX_PIN 2
#define CAM_TX_PIN 3

//Set the timer
#define SNAPSHOT_TIMER 60*60*1000 //Every hour
time_t t;


// TODO: Finish implementing sending command buffer
String cmdBuf = "";
boolean cmdComplete = false;


// Serial and camera connections
SoftwareSerial cameraconnection = SoftwareSerial(CAM_RX_PIN,CAM_TX_PIN);
Adafruit_VC0706 cam = Adafruit_VC0706(&cameraconnection);


// Writes a node command to serial 
void printCommand(char* cmd, char* param){

}

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
    Serial.println("AV+ERR,0x02");
    return;
  }
  
  // Get the size of the image (frame) taken  
  uint16_t jpglen = cam.frameLength();

  File imgFile;
  
  // Create an image with the name IMAGExx.JPG
  char filename[13];
  
  strcpy(filename, "IMAGE00.JPG");

  if(saveToSD){
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
  
  //Signal the start of the image
  Serial.print("{");
  Serial.print("+AV+CTRANS,");
  Serial.print(jpglen);
  Serial.print("}");
  
  while (jpglen > 0) {
    
    // read IMG_BUF_SIZE bytes at a time;
    uint8_t *buffer;
    uint8_t bytesToRead = min(IMG_BUF_SIZE, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
    buffer = cam.readPicture(bytesToRead);

    if(saveToSD) imgFile.write(buffer, bytesToRead);
    
    Serial.print("{");
    Serial.write(buffer,bytesToRead);
    Serial.print("}");

    time_t t = millis();
    uint8_t counter = 0;
    
    while((counter < 3) && ((millis()-t)<5000)){
      
      if(Serial.available()){
        if((char)Serial.read() == '+'){
          counter += 1;
        }
      }
        
    }

    counter = 0;
    jpglen -= bytesToRead;
  }
  
  if(saveToSD) imgFile.close();
  Serial.print("{");
  Serial.print("+++");
  Serial.print("}");
  
}


void setup() {
  
  if(chipSelect != 10) pinMode(10, OUTPUT); // SS on Uno, etc.

  Serial.begin(115200);
  //Serial.println("AV+ON,node_id");
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("AV+ERR,0x01");
    // don't do anything more:
    //return;
  }  
  
  // Try to locate the camera
  if (cam.begin()) {
    Serial.println("{AV+CS,0x01}");
  } else {
    Serial.println("AV+CS,0x00");
    return;
  }


  // Set the picture size - you can choose one of 640x480, 320x240 or 160x120 
  // Remember that bigger pictures take longer to transmit!
  
  cam.setImageSize(VC0706_640x480);        // biggest
  //cam.setImageSize(VC0706_320x240);        // medium
  //cam.setImageSize(VC0706_160x120);          // small


  // Send snapshot every period
  //t.setInterval(SNAPSHOT_PERIOD,sendSnapshot);

  // Set the timer
  t = millis();

  cmdBuf.reserve(ND_CMD_BUF_SIZE);

  
}

void loop(){

    if(cmdComplete){

      cmdBuf.trim();
      
      if(cmdBuf.equals("AV+JBRD"))
        Serial.println("+RRBDRES:00FF214E2418");
      else if(cmdBuf.equals("AV+CGETS"))
        sendSnapshot(true);
      else if(cmdBuf.equals("AV+JRES"))
        Serial.println("ROK");
      else if(cmdBuf.equals("OK")){
        
      }
      else{
        Serial.print("NOK: ");
        Serial.println(cmdBuf);
      }
  
      cmdBuf = "";
      cmdComplete = false;
  }


}

// Thread for processing serial events
void serialEvent(){
  while(Serial.available()){
    char inChar = (char)Serial.read();
    cmdBuf += inChar;

    if(inChar == '\n'){
      cmdComplete = true;
    }

    //Serial.print("RX: ");
    //Serial.print(inChar);
    //Serial.println("");
  }
}
