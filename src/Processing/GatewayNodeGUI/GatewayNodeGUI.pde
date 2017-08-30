

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


// Window parameters
int bgColor = 200;

// Button Parameters
int btnWidth = 100;
int btnHeight = 30;

// GUI Controls
GUIController c;
IFLookAndFeel defaultLook;
IFButton getSnapshotBtn;
IFProgressBar progress;
IFLabel progressLbl;
String outputText ="";

// File IO Variables


// Serial parameters
final String SER_PORT = "COM5";
final int BUF_SIZE = 64;
final int SER_BAUD_RATE = 57600;
Serial gwSerial;                       

// Command Buffer
String cmdBuf = "";
boolean cmdComplete = false;
boolean imgRead;

// Snapshot Image File Parameters

int totalFileSize = 0;
int currentFileSize = 0;
int extraBytes = 0;
int lastcount = -1;

PImage imgFile;
OutputStream imgWriter;
String recvImgFileName="";


void setup() {
  
  size(1200, 600);
  frameRate(60);
  background(bgColor);
 
  // Serial initialization
  
  
  // Uncomment next two lines of code to determine COM port element in Serial array
  // printArray(Serial.list());
  // exit();
  
  gwSerial = new Serial(this, Serial.list()[2], SER_BAUD_RATE);
  
  // Create GUI

  c = new GUIController (this);

  // xpos, ypos, height, width
  int xpos_getSnapshotBtn = int(width*0.05); //int(width/2-btnWidth/2);
  int ypos_getSnapshotBtn = int(height*0.1);
  
  int xpos_progressBar = int(xpos_getSnapshotBtn+btnWidth+10);
  int ypos_progressBar = int(ypos_getSnapshotBtn+btnHeight/4);
  
  int xpos_progressLbl = xpos_progressBar;
  int ypos_progressLbl = ypos_progressBar-20;
  
  try{
    getSnapshotBtn = new IFButton ("Take Snapshot", xpos_getSnapshotBtn, ypos_getSnapshotBtn, btnWidth, btnHeight);
    getSnapshotBtn.addActionListener(this);
    
    progress = new IFProgressBar (xpos_progressBar, ypos_progressBar, int(width*0.4));
    progressLbl = new IFLabel("Transfer Progress",xpos_progressLbl,ypos_progressLbl, 12);
    
    imgFile = loadImage(sketchPath() +"/IMAGE00.jpg");
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
  
  println("Image width x height: "+str(imgFile.width)+" x " + str(imgFile.height));
  println("Adjusted image width x height: "+str(imgFile.width*0.15)+" x " + str(imgFile.height*0.15));
  
}


// Main program draw loop 

void draw() {
  
  background(200);

  //float imageReduction = 0.15;
  
  int imgSizeWidth =  640;
  int imgSizeHeight = 480;
  
  
  
  // Create image file 
  image(imgFile, int(width-imgSizeWidth*1.5), int(height-imgSizeHeight*1.05), imgSizeWidth, imgSizeHeight);
  
  textSize(12);
  text(outputText,50,200);
  textAlign(LEFT,CENTER);
  fill(50);
  
  totalFileSize = (totalFileSize == 0)?1:totalFileSize;
  
  // File transfer progress status
  progress.setProgress(norm(currentFileSize, 0,totalFileSize));
  progressLbl.setLabel("Transfer Progress "+str(round(progress.getProgress()*100))+"%"+" File Size: "+str(float(totalFileSize/1024))+"kb or "+str(totalFileSize)+" bytes");
  
  if(cmdComplete){
    
    if(!imgRead){
    
     cmdBuf.trim();
     println("cmdBuf recv: " + cmdBuf.trim());
     String[] m = match(cmdBuf.trim(), "AV\\+(.*?);");
     
     if(m != null) {
       String[] tkn = splitTokens(m[1], ", ");
       
       if(match(tkn[0],"CS") != null){ 
         outputText = "Connected to Gateway";      // AV+CS,0x01;
       
       }else if(match(tkn[0],"CTRANS") != null) {                           // AV+CTRANS,<img_file_size_in_bytes>;  Start image transfer
         totalFileSize = int(tkn[1]);
         outputText = "Getting Image..." ;
         imgRead = true;
       
       }else if(match(tkn[0],"DEBUG") != null){
         
         if(match(tkn[1],"TAKESNAP_SD") != null){
           println("Snapshot taken.  Arduino filename: "+tkn[2]+" Filesize: "+tkn[3]+" bytes");
           
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
  
  gwSerial.write("AV+CGETS");
  gwSerial.write(10); 
  
  recvImgFileName = String.valueOf(year());
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
    outputText = "Waiting for image data...";
    snapshotCmd();
  } 
}

void serialEvent(Serial s){

    if(cmdComplete) return;
    
    // If image reading flag active, read serial TX as byte stream
    
    if(imgRead){
      
        if((totalFileSize-currentFileSize) > 0 ){
    
            try{
              
              int bytesToRead = min(64, (totalFileSize-currentFileSize));
              byte[] buffer = new byte[bytesToRead];
              
              buffer = s.readBytes(bytesToRead);
              imgWriter.write(buffer);
              
              currentFileSize+=buffer.length;
              buffer = null;

            }
            catch(IOException e){
              println("IOException");
            }
            
            
            if(((currentFileSize % 128) == 0) && currentFileSize > 40000)
              println(" Filesize: "+str(currentFileSize)+" Current min: "+str(min(64, (totalFileSize-currentFileSize))));  
         
       
          if((totalFileSize-currentFileSize) == 0){
             
             try{
               imgWriter.flush();
               imgWriter.close();
             }
             catch(IOException e){
               println("Exception on file close");
               imgRead = false;
               return;
             }
             
             outputText = "Transfer complete. File: "+recvImgFileName;
             
             imgFile = loadImage(sketchPath() + "/" + recvImgFileName);
             
             println("New image width x height: "+str(imgFile.width)+" x " + str(imgFile.height));
             
             imgRead = false;
     
          }
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