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

#define node_id = 42134423
#define ND_CMD_BUF_SIZE 200

#define IMG_BUF_SIZE 64

// SD card chip select
#define chipSelect 4

// Camera pins
#define CAM_RX_PIN 2
#define CAM_TX_PIN 3

//Set the timer
#define SNAPSHOT_TIMER 60*60*1000 //Every hour
time_t t;


// TODO: Finish implementing sending command buffer
String cmdBuf;
boolean cmdComplete = false;


// Serial and camera connections
SoftwareSerial cameraconnection = SoftwareSerial(CAM_RX_PIN,CAM_TX_PIN);
Adafruit_VC0706 cam = Adafruit_VC0706(&cameraconnection);


// Writes a node command to serial 
void sendCommand(char* cmd, char* param){
  
  

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
  
  if(chipSelect != 10) pinMode(10, OUTPUT); // SS on Uno, etc.

  Serial.begin(115200);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("AV+ERR,0x01;");
    // don't do anything more:
    //return;
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
        sendSnapshot(false);
        //Serial.println("+AV+CTRANS,43000;");
      else if(cmdBuf.equals("AV+JRES"))
        Serial.println("ROK");
      else if(cmdBuf.equals("OK")){
        
      }
      else{
        Serial.print("NOK: ");
        Serial.println(cmdBuf);
        Serial.write('\n');
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
