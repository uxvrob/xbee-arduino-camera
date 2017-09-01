/***************************************************************
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
 
final String SER_PORT = "COM3";       //Serial.list()[2];
final int SER_BAUD_RATE = 57600;      //This should match the Serial baud rate in the GatewayNode firmware
final int BUF_SIZE = 32;              

// Window parameters

final int bgColor = 200;

// Button Parameters

final int btnWidth = 100;
final int btnHeight = 30;

// Image pixel size in GUI

final int imgSizeWidth =  640;
final int imgSizeHeight = 480;

// Image streaming timeout

final int TIMEOUT = 5000;  // in milliseconds (ms)

// GUI Controls - Interfascia
GUIController c;
IFLookAndFeel defaultLook;
IFButton getSnapshotBtn;
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

Serial gwSerial;

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
  
  // printArray(Serial.list());
  // exit();
 
  try{
    gwSerial = new Serial(this, SER_PORT, SER_BAUD_RATE);
    
    println("Serial port: " + SER_PORT);

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
    getSnapshotBtn = new IFButton ("Take Snapshot", xpos_getSnapshotBtn, ypos_getSnapshotBtn, btnWidth, btnHeight);
    getSnapshotBtn.addActionListener(this);
    
    progress = new IFProgressBar (xpos_progressBar, ypos_progressBar, int(width*0.4));
    progressLbl = new IFLabel("Transfer Progress",xpos_progressLbl,ypos_progressLbl, 12);
    
    imgFile = loadImage(sketchPath() +"/default.jpg");
  }
  catch(Exception e){
    println ("Unhandeled exception");
    exit();
  }
  
  c.add (getSnapshotBtn);
  c.add (progress);
  c.add (progressLbl);

  defaultLook = new IFLookAndFeel(this, IFLookAndFeel.DEFAULT);
  
  c.setLookAndFeel(defaultLook);
  
  txtAConsole.setText("Actual Image width x height: "+str(imgFile.width)+" x " + str(imgFile.height)
                      +"\nAdjusted image width x height: "+str(int(imgFile.width*0.15))+" x " + str(int(imgFile.height*0.15))
                    );
                    
  txtAConsole.setText(txtAConsole.getText()+"\n"
                      + "Ready to take snapshot!\ns");
                      
  progress.setProgress(norm(currentFileSize, 0,totalFileSize));
  progressLbl.setLabel("Transfer Progress "+str(round(progress.getProgress()*100))+"%"+" File Size: "+str(float(totalFileSize/1024))+"kb or "+str(totalFileSize)+" bytes");
  
}


// Main program draw loop 

void draw() {
  
  background(200);
 
  int xpos_imgFile = int(width-imgSizeWidth*1.5+100);
  int ypos_imgFile = int(height-imgSizeHeight*1.05);
  
  // Create image file 
  image(imgFile, xpos_imgFile, ypos_imgFile, imgSizeWidth, imgSizeHeight);
  
  // Re-position controls
  
  getSnapshotBtn.setPosition(xpos_imgFile, ypos_imgFile - getSnapshotBtn.getHeight()-10);
  
  progress.setPosition(getSnapshotBtn.getX()+getSnapshotBtn.getWidth()+10, int(getSnapshotBtn.getY()+getSnapshotBtn.getHeight()/4));
  
  progressLbl.setPosition(progress.getX(),progress.getY()-20);
  
  txtAConsole.setPosition(txtAConsole.getPosition()[0],getSnapshotBtn.getY());
  txtAConsole.setSize(txtAConsole.getWidth(), getSnapshotBtn.getHeight() + imgSizeHeight+5);
  
  textSize(32);
  text("Smartifi Console", txtAConsole.getPosition()[0], txtAConsole.getPosition()[1]-20); 
  fill(0, 102, 153, 51);
  
  if(keyPressed && key==' ') {
    txtAConsole.scroll((float)mouseX/(float)width);
  }
  if(keyPressed && key=='l') {
    txtAConsole.setLineHeight(mouseY);
  }
  
  totalFileSize = (totalFileSize == 0)?1:totalFileSize;
  
  // File transfer progress status
  
  if(imgRead){
    if((millis()-timer)>=TIMEOUT){
      txtAConsole.setText(txtAConsole.getText()
                                   + "Gateway timeout on image transfer. Time: " + str(timer) + " ms\n");
                                   
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
    }
    else if((currentFileSize == totalFileSize)){
      progress.setProgress(norm(currentFileSize, 0,totalFileSize));
      progressLbl.setLabel("Transfer Complete! "+str(round(progress.getProgress()*100))+"%"+" File Size: "+str(float(totalFileSize/1024))+"kb or "+str(totalFileSize)+" bytes");
    }
    else{
        progress.setProgress(norm(currentFileSize, 0,totalFileSize));
        progressLbl.setLabel("Transfer Progress "+str(round(progress.getProgress()*100))+"%"+" File Size: "+str(float(totalFileSize/1024))+"kb or "+str(totalFileSize)+" bytes");
    }
  }
  
  
  // COMMAND PROCESSING
  
  if(cmdComplete){
    
    if(!imgRead){
    
     cmdBuf.trim();
     println("cmdBuf recv: " + cmdBuf.trim());
     String[] m = match(cmdBuf.trim(), "AV\\+(.*?);");
     
     if(m != null) {
       String[] tkn = splitTokens(m[1], ", ");
       
       if(match(tkn[0],"CS") != null){ 
         
         txtAConsole.setText(txtAConsole.getText()
                      + "Connected to Gateway\n");      // AV+CS,0x01;
       
       }else if(match(tkn[0],"CTRANS") != null) {                           // AV+CTRANS,<img_file_size_in_bytes>;  Start image transfer
         totalFileSize = int(tkn[1]);
         txtAConsole.setText(txtAConsole.getText() + 
                             "Getting Image...\n");
                             
         // Switch to reading image byte stream in Serial Event handler and start timeout timer
         imgRead = true;
         timer = millis();
       
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
     cmdComplete = false;
     cmdBuf="";
     
  }
 
}

void snapshotCmd(){
  
  //gwSerial.write("AV+CGETS");
  //gwSerial.write("AV+SGETS");
  gwSerial.write("AV+CSEND");
  gwSerial.write(10); 
  
  recvImgFileName += "IMAGE_";
  recvImgFileName += String.valueOf(year());
  recvImgFileName += String.valueOf(month());
  recvImgFileName += String.valueOf(day());
  recvImgFileName += "-";
  recvImgFileName += String.valueOf(hour());
  recvImgFileName += String.valueOf(minute());
  recvImgFileName += ".jpg";
  
  imgWriter = createOutput(sketchPath()+"/"+recvImgFileName);
  currentFileSize =0;
  totalFileSize=0;

}

void actionPerformed (GUIEvent e) {
  if (e.getSource() == getSnapshotBtn) {

      txtAConsole.setText(txtAConsole.getText() 
                          + "\nWaiting for image data...\n");
                          
      snapshotCmd();

  } 
}

void serialEvent(Serial s){

    if(cmdComplete) return;
    
    // If CTRANS command received, read byte stream until stream is completed.
    
    if(imgRead){
      
        // There is a delay because image is being saved to the SD card first then transmitted.
        
        if((totalFileSize-currentFileSize) > 0 ){
            

            
            // Read stream data and output to jpg file
            try{
              
              int bytesToRead = min(BUF_SIZE, (totalFileSize-currentFileSize));
              byte[] buffer = new byte[bytesToRead];
              
              buffer = s.readBytes(bytesToRead);
              imgWriter.write(buffer);
              if(buffer.length > 0){                
                timer = millis();
              }
              currentFileSize+=buffer.length;
              buffer = null;

            }
            catch(IOException e){
              e.printStackTrace();
              txtAConsole.setText(txtAConsole.getText()
                + "Exception on serial read\n");
            }
            
            // Output a response every 128 bytes
            if(((currentFileSize % 128) == 0) /*&& currentFileSize > 40000*/){
              //txtAConsole.setText(txtAConsole.getText()
                      println("Current Filesize: "+str(currentFileSize)+" Buffer size: "+str(min(64, (totalFileSize-currentFileSize))) + "\n");
              
            }
        }
       else if((totalFileSize == currentFileSize)){
             
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
                                 + "New image width x height: "+str(imgFile.width)+" x " + str(imgFile.height)+ "\n");
             }
             catch(Exception e){
                 e.printStackTrace();
                 txtAConsole.setText(txtAConsole.getText()
                               +"Could not load image... currentFileSize: "+currentFileSize + "\n");
  
                 imgFile = loadImage(sketchPath() +"/default.jpg");
             }

             imgRead = false;
     
          }
    
   }
   else{
      char inChar = s.readChar();
      cmdBuf += str(inChar);
      
      if(inChar == ';'){
          cmdComplete = true;   
      }
      
      if(cmdBuf.length() > 100){
        cmdBuf="";
      }
        
     
   }
    
}