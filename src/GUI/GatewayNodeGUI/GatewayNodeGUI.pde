/*************************************************************** //<>// //<>//
* Gateway Node GUI 
*
* @author Robbie Sharma robbie -at- rsconsulting.ca
* @date August 21, 2017
* @desc GUI for direct file transfer from GUI
* 
***************************************************************/

import controlP5.*;
import interfascia.*;
import processing.serial.*;

// Serial parameters
/********************************************************************
 * Determine active serial port in Serial.list()[x] for port refernece
 * by uncommenting printArray block in setup() method
 *******************************************************************/

// Camera Node - COM3
// Gateway Node - COM4

final String SER_PORT   = "COM4";       //Serial.list()[2];
final int SER_BAUD_RATE = 57600;      //This should match the Serial baud rate in the GatewayNode firmware
final int BUF_SIZE      = 64;              

// Window parameters

final int bgColor = 200;

// Button Parameters

final int btnWidth = 100;
final int btnHeight = 30;

// Image pixel size in GUI

final int imgSizeWidth =  640;
final int imgSizeHeight = 480;

// Image streaming timeout

final int TIMEOUT = 30000;  // in milliseconds (ms)

// GUI Controls - Interfascia
GUIController c;
IFLookAndFeel defaultLook;
IFButton getSnapshotBtn;
IFButton getRecentSnapBtn;
IFProgressBar progress;
IFLabel progressLbl;

// GUI Controls - ControlP5
ControlP5 cp5;
Textarea txtAConsole;

// Command Parser/Buffer

String cmdBuf = "";
boolean cmdComplete = false;
boolean imgRead;

// Snapshot Image File Parameters

PImage imgFile;
OutputStream imgWriter;
String recvImgFileName="";

int totalFileSize = 0;
int currentFileSize = 0;
int timer = 0;
int errCount = 0;
char response;

int bufferIndex = 0;
int bytesToRead = 0;
byte[] buffer;

int u32StartTime = 0;

Serial gwSerial;

/******************************************************
* Exception Codes
*******************************************************/
final char ku8MBSuccess               = 0x00;
final char ku8MBIllegalFunction       = 0x01;
final char ku8MBIllegalDataAddress     = 0x02;
final char ku8MBIllegalDataValue     = 0x03;
final char ku8MBSlaveDeviceFailure     = 0x04;
final char ku8MBInvalidSlaveID       = 0xE0;
final char ku8MBInvalidFunction       = 0xE1;
final char ku8MBResponseTimedOut     = 0xE2;
final char ku8MBInvalidCRC         = 0xE3;

void setup() {
  
  size(1200, 600);
  frameRate(60);
  background(bgColor);
 
  // Serial initialization
  
  /********************************************************************************
   * Uncomment next two lines of code, then run, to determine COM port element 
   * in Serial array.  Look at the console output for the revelent com port
   * assigned to the corresponding array element.  Change SER_PORT var with correct
   * array element.
  *********************************************************************************/
  
  println("COM Ports active");
  printArray(Serial.list());
  //exit();
 
  try{
    gwSerial = new Serial(this, SER_PORT, SER_BAUD_RATE);
    buffer = new byte[BUF_SIZE+5];
    println("Connected to serial port: " + SER_PORT);

  }
  catch(RuntimeException e){
    e.printStackTrace();
    println("Could not open "+ SER_PORT + "... exiting...");
    exit();
  }
  
  
  // Create GUI controllers

  c = new GUIController (this);
  cp5 = new ControlP5(this);

  // xpos, ypos, height, width
  
  int xpos_getSnapshotBtn = int(width*0.05); //int(width/2-btnWidth/2);
  int ypos_getSnapshotBtn = int(height*0.1);
  
  int xpos_getRecentSnapBtn = xpos_getSnapshotBtn;
  int ypos_getRecentSnapBtn = ypos_getSnapshotBtn-btnHeight-10;
  
  int xpos_progressBar = int(xpos_getSnapshotBtn+btnWidth+10);
  int ypos_progressBar = int(ypos_getSnapshotBtn+btnHeight/4);
  
  int xpos_progressLbl = xpos_progressBar;
  int ypos_progressLbl = ypos_progressBar-20;
  
  int xpos_txtAConsole = 50;
  int ypos_txtAConsole = 50;
  
  txtAConsole = cp5.addTextarea("txt")
                  .setPosition(xpos_txtAConsole,ypos_txtAConsole)
                  .setSize(270,320)
                  .setFont(createFont("arial",12))
                  .setLineHeight(14)
                  .setColor(color(128))
                  .setColorBackground(color(255,100))
                  .setColorForeground(color(255,100))
                  .showScrollbar();
                 
                
  try{
    getSnapshotBtn = new IFButton ("Take Snapshot and Get", xpos_getSnapshotBtn, ypos_getSnapshotBtn, btnWidth, btnHeight);
    getSnapshotBtn.addActionListener(this);
    
    getRecentSnapBtn = new IFButton ("Get Recent Snapshot", xpos_getRecentSnapBtn, ypos_getRecentSnapBtn, btnWidth, btnHeight);
    getRecentSnapBtn.addActionListener(this);
    
    progress = new IFProgressBar (xpos_progressBar, ypos_progressBar, int(width*0.4));
    progressLbl = new IFLabel("Transfer Progress",xpos_progressLbl,ypos_progressLbl, 12);
    
    imgFile = loadImage(sketchPath() +"/default.jpg");
  }
  catch(Exception e){
    println ("Unhandeled exception");
    exit();
  }
  
  c.add (getSnapshotBtn);
  c.add (getRecentSnapBtn);
  c.add (progress);
  c.add (progressLbl);

  defaultLook = new IFLookAndFeel(this, IFLookAndFeel.DEFAULT);
  
  c.setLookAndFeel(defaultLook);
  
  txtAConsole.setText("Actual Image width x height: "+str(imgFile.width)+" x " + str(imgFile.height)
                      +"\nAdjusted image width x height: "+str(int(imgFile.width*0.15))+" x " + str(int(imgFile.height*0.15))
                    );
                    
  txtAConsole.setText(txtAConsole.getText()+"\n"
                      + "\nPress RESET on the Camera Node to SYNC!\n\n");
                      
  progress.setProgress(norm(currentFileSize, 0,totalFileSize));
  progressLbl.setLabel("Transfer Progress "+str(round(progress.getProgress()*100))+"%"+" File Size: "+str(float(totalFileSize/1024))+"kb or "+str(totalFileSize)+" bytes");
  
}


// Main program draw loop 

void draw() {
  
   /**********************************************************
  ***************** SETUP GUI AND DRAW IMAGE *****************
  **********************************************************/
  
  background(200);
 
  int xpos_imgFile = int(width-imgSizeWidth*1.5+100);
  int ypos_imgFile = int(height-imgSizeHeight*1.05);
  
  // Create image file 
  
  image(imgFile,xpos_imgFile,ypos_imgFile, imgSizeWidth, imgSizeHeight);
  
   /**********************************************************
  ******************** SETUP CONTROLS ************************
  **********************************************************/
  
  getSnapshotBtn.setPosition(xpos_imgFile, ypos_imgFile - getSnapshotBtn.getHeight()-10); 
  
  getRecentSnapBtn.setPosition(getSnapshotBtn.getX(),getSnapshotBtn.getY()-(btnHeight+10));
  
  progress.setPosition(getSnapshotBtn.getX()+getSnapshotBtn.getWidth()+10, int(getSnapshotBtn.getY()+getSnapshotBtn.getHeight()/4));
  
  progressLbl.setPosition(progress.getX(),progress.getY()-20);
  
  txtAConsole.setPosition(txtAConsole.getPosition()[0],getSnapshotBtn.getY());
  txtAConsole.setSize(txtAConsole.getWidth(), getSnapshotBtn.getHeight() + imgSizeHeight+5);
  
  
   /**********************************************************
  ******************** TITLE *********************************
  **********************************************************/
  textSize(32);
  text("Smartifi Console", txtAConsole.getPosition()[0], txtAConsole.getPosition()[1]-20); 
  fill(0, 102, 153, 51);
  
  if(keyPressed && key==' ') {
    txtAConsole.scroll((float)mouseX/(float)width);
  }
  if(keyPressed && key=='l') {
    txtAConsole.setLineHeight(mouseY);
  }
 
 /**********************************************************
  ******************** PROGRESS BAR ************************
  **********************************************************/
 
  totalFileSize = (totalFileSize == 0)?1:totalFileSize;
   
  if(imgRead){

    if((millis()-timer)>=TIMEOUT){
      txtAConsole.setText(txtAConsole.getText()
                                   + "Gateway timeout on image transfer. Time: " + str(timer) + " ms\n"
                                   + "Received File size: "+str(currentFileSize)+" Total File Size: "+ str(totalFileSize)+"\n\n");
      
      try{
         imgWriter.flush();
         imgWriter.close();
       }
       catch(IOException e){
         e.printStackTrace();
         txtAConsole.setText(txtAConsole.getText()+
                             "Exception generated on file close...\n");
                            
       }
               
      
          progress.setProgress(norm(currentFileSize, 0,totalFileSize));
          progressLbl.setLabel("Transfer Timeout Occurred! "+str(round(progress.getProgress()*100))+"%"+" File Size: "+str(float(totalFileSize/1024))+"kb or "+str(totalFileSize)+" bytes");
    
        
        timer=0;
        imgRead = false;
        cmdComplete=false;
    }
    else if((currentFileSize == totalFileSize)){
              /**********************************************************
              ******************** IMG TRANSFER COMPLETE*****************
              **********************************************************/
             try{
               imgWriter.flush();
               imgWriter.close();
             }
             catch(IOException e){
               e.printStackTrace();
               txtAConsole.setText(txtAConsole.getText()+
                                   "Exception generated on file close...\n");
                                   
               imgRead = false;
               return;
             }
             

             txtAConsole.setText(txtAConsole.getText()
                                 +"Transfer complete. File: "+recvImgFileName + "\n");
             try{
               imgFile = loadImage(sketchPath() + "/" + recvImgFileName);
               txtAConsole.setText(txtAConsole.getText()
                                 + "New image width x height: "+str(imgFile.width)+" x " + str(imgFile.height)+ "\n\n");
             }
             catch(Exception e){
                 e.printStackTrace();
                 txtAConsole.setText(txtAConsole.getText()
                               +"Could not load image... currentFileSize: "+currentFileSize + "\n\n");
  
                 imgFile = loadImage(sketchPath() +"/default.jpg");
             }

             imgRead = false;
      
      progress.setProgress(norm(currentFileSize, 0,totalFileSize));
      progressLbl.setLabel("Transfer Complete! "+str(round(progress.getProgress()*100))+"%"+" File Size: "+str(float(totalFileSize/1024))+"kb or "+str(totalFileSize)+" bytes");
    }
    else{
        progress.setProgress(norm(currentFileSize, 0,totalFileSize));
        progressLbl.setLabel("Transfer Progress "+str(round(progress.getProgress()*100))+"%"+" File Size: "+str(float(totalFileSize/1024))+"kb or "+str(totalFileSize)+" bytes");
    }
  }
  
  
   /**********************************************************
  ******************** SERIAL PROCESSING *********************
  **********************************************************/
  
  if(cmdComplete){
    
    if(!imgRead){
    
     cmdBuf.trim();
     println("cmdBuf recv: " + cmdBuf.trim());
     String[] m = match(cmdBuf.trim(), "AV\\+(.*?);");
     
     if(m != null) {
       String[] tkn = splitTokens(m[1], ", ");
       
       if(match(tkn[0],"CS") != null){ 
         
         
         
           txtAConsole.setText(txtAConsole.getText()
                        + "Connected to Camera Node\n");      // AV+CS,0x01;
                        
           if(match(tkn[1],"0x01") != null){
             
             txtAConsole.setText(txtAConsole.getText()
                        + "Camera is active and ready to take snaps!\n\n"); 
             
           }
           else{
             
             txtAConsole.setText(txtAConsole.getText()
                        + "Camera is having problems... is it plugged in properly?\n"
                        + "Press RESET on Camera Node after fixing\n\n");
           }
          
       
       }else if(match(tkn[0],"CTRANS") != null) {                           // AV+CTRANS,<img_file_size_in_bytes>;  Start image transfer
         totalFileSize = int(tkn[1]);
         println("MATCH: TOTALFILESIZE",totalFileSize);
         txtAConsole.setText(txtAConsole.getText() + 
                             "Getting Image of size: " + totalFileSize + "...\n");
                             
         createImageFile(); 
         // Switch to reading image byte stream in Serial Event handler and start timeout timer
         imgRead = true;
         timer = millis();
         sendGatewayCmd("+");
       
       }else if(match(tkn[0],"DEBUG") != null){
         
         if(match(tkn[1],"TAKESNAP_SD") != null){
           txtAConsole.setText(txtAConsole.getText()
                               + "Snapshot taken!\n"
                               + "Arduino filename: "+tkn[2]+" Filesize: "+tkn[3]+" bytes\n");
           
         }
         
       }
      
       println("Match: "+m[1]);
     }
    }
    else if(imgRead){
       printExceptionCode(response);
       
       switch (response){   
          case ku8MBSuccess:
            sendGatewayCmd("+");
            
            break;
          case ku8MBIllegalFunction:
          case ku8MBIllegalDataAddress:     
          case ku8MBIllegalDataValue:   
          case ku8MBSlaveDeviceFailure: 
          case ku8MBInvalidSlaveID:       
          case ku8MBInvalidFunction:      
          case ku8MBResponseTimedOut:    
          case ku8MBInvalidCRC:
            gwSerial.clear();
            delay(10);
            sendGatewayCmd("-");
            break;
       }
       
       response = ku8MBSuccess;
       bytesToRead = min(BUF_SIZE, (totalFileSize-currentFileSize))+5;
       
       bufferIndex = 0;
       u32StartTime = millis();
       
    }
    cmdComplete = false;
    cmdBuf="";
  }
  
 
}



void sendGatewayCmd(String cmd){
  
  gwSerial.write(cmd);
  gwSerial.write(10); 

}

void createImageFile(){
  
  recvImgFileName = "IMAGE_";
  recvImgFileName += String.valueOf(year());
  recvImgFileName += String.valueOf(month());
  recvImgFileName += String.valueOf(day());
  recvImgFileName += "-";
  recvImgFileName += String.valueOf(hour());
  recvImgFileName += String.valueOf(minute());
  recvImgFileName += ".jpg";
  
  imgWriter = createOutput(sketchPath()+"/"+recvImgFileName);
  currentFileSize =0;
  
}

void actionPerformed (GUIEvent e) {
  
  if (e.getSource() == getSnapshotBtn) {

      txtAConsole.setText(txtAConsole.getText() 
                          + "Waiting for image data...\n"
                          + "Grab a coffee... this will take awhile..\n");
      
      
      //sendGatewayCmd("AV+CGETS");
      sendGatewayCmd("AV+SGETS");
      
      
      

  } 
  
  if(e.getSource() == getRecentSnapBtn){

    
    txtAConsole.setText(txtAConsole.getText() 
                          + "Getting most recent image stored...\n");
    
    sendGatewayCmd("AV+CSEND");
     
  }
}



int word(byte high, byte low) {
  int ret_val = low;
  ret_val += (high << 8);
  return ret_val;
}



void printExceptionCode(char e){
  
  switch(e){
    case ku8MBIllegalFunction:
      println("Illegal function");
      break;
    case ku8MBIllegalDataValue:
      println("Illegal Data Value");
      break;
    case ku8MBInvalidCRC:
      println("Invalid CRC");
      break;
    case ku8MBResponseTimedOut: 
    case ku8MBIllegalDataAddress: 
    case ku8MBSlaveDeviceFailure:
    case ku8MBInvalidSlaveID:       
    case ku8MBInvalidFunction:      
       println("Invalid response");
       break;
    
  }
  
}

int _crc16_update(int crc, byte a) {

  crc ^= a;
  for (int i = 0; i < 8; ++i)
  {
    if ((crc & 1)==1)
      crc = (crc >> 1) ^ 0xA001;
    else
      crc = (crc >> 1);
  }
  return crc;
}

void processImgInput(Serial s){    
        
    while(bytesToRead >=0 && (response != ku8MBSuccess)){
      
          if(s.available()>0){
              
              buffer[bufferIndex++] = byte(s.read());
              if(buffer[0] == 0){  // discard any 0 bytes;
                  bufferIndex--;   
                  return;
              }
              
              bytesToRead--;

          }else{
             delay(10); 
          }
          
          if(bufferIndex == 2){
            if(buffer[0] != 0x76) response = ku8MBIllegalFunction; 
            if(int(buffer[1]) != (bytesToRead-3)) {
              //println("Expected: ",bytesToRead-3,"Received: ",buffer[1]);
              response = ku8MBIllegalDataValue;
              cmdComplete = true;
              return;
            }
          }
          
          if (millis() > (u32StartTime + 1000)) {
            response = ku8MBResponseTimedOut;
            println("Timed out");
            cmdComplete = true;
            return;
          }
          
          
     }
     
     if(response == ku8MBSuccess && bufferIndex >= 2){
           /*     
           int u16CRC = 0xFFFF;
            
            for(int i=0; i < int(buffer[1]); i++){
              print(i,":",hex(buffer[i+2]), " ");
              u16CRC = _crc16_update(u16CRC, buffer[i+2]);
            }
            
            if ((u16CRC != word(buffer[bufferIndex - 3], buffer[bufferIndex - 2]))) {
                response = ku8MBInvalidCRC;
                println("invalid crc calculated:",hex(u16CRC),"received high: ",hex(buffer[bufferIndex - 3]), "low: ", hex(buffer[bufferIndex - 2]));
            }
            else println("valid CRC");
            
            */
            
            if((int(buffer[1]) != int(buffer[bufferIndex-3])) && (int(buffer[1]) != int(buffer[bufferIndex-2]))){
                println("invalid crc calculated:",int(buffer[1]),"received high: ",int(buffer[bufferIndex - 3]), "low: ", int(buffer[bufferIndex - 2]));
            
              response = ku8MBInvalidCRC;
              cmdComplete = true;
            }            
     }
          
     if((totalFileSize-currentFileSize) > 0  && response == ku8MBSuccess){
  
              
              // Read stream data and output to jpg file
              println ("writing image data currentFileSize: ", currentFileSize);
              try{
                
                for(int i=0; i < int(buffer[1]); i++){
                  imgWriter.write(buffer[i+2]);
                }
                
                currentFileSize+=(int(buffer[1]));
                timer = millis(); // Timer reset
  
              }
              catch(IOException e){
                e.printStackTrace();
                txtAConsole.setText(txtAConsole.getText()
                  + "Exception on serial read\n");
              }
              
              // Output a response every 128 bytes
              if(((currentFileSize % 128) == 0) && currentFileSize > 20000){
                        println("Current Filesize: "+str(currentFileSize)+" Buffer size: "+str(min(64, (totalFileSize-currentFileSize))) + "\n");
                
              }
              
              cmdComplete = true;
     }
   
     
     return;

}

void serialEvent (Serial s){
    
    if(imgRead) {
      processImgInput(s);
    }
    
   else if(!imgRead){
      char inChar = s.readChar();
      cmdBuf += str(inChar);
      
      if(inChar == ';'){
          cmdComplete = true;
          println("");
      }
      
      if(cmdBuf.length() > 100){
        cmdBuf="";
      }
      
      print (inChar);
   }
    
}