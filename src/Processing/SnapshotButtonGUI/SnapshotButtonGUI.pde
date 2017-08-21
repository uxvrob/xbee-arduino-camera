import interfascia.*;
import processing.serial.*;

import java.io.DataOutputStream;
import java.io.BufferedOutputStream;
import java.io.FileOutputStream;

// Window parameters
int bgColor = 200;

// Btn Parameters
int btnWidth = 100;
int btnHeight = 30;

// GUI Variables
GUIController c;
IFLookAndFeel defaultLook;
IFButton getSnapshotBtn;
IFProgressBar progress;
IFLabel progressLbl;


// File IO Variables

  FileOutputStream fstream; 
  BufferedOutputStream bstream; 
  DataOutputStream dstream; 

// Serial variables
final int BUF_SIZE = 64;
Serial gwSerial;                       // The serial port

String cmdBuf = "";
boolean cmdComplete = false;
boolean imgRead;

int totalFileSize = 0;
int currentFileSize = 0;
int extraBytes = 0;
int lastcount = -1;

String outputText ="";

PImage imgFile;
OutputStream imgWriter;


void setup() {
  
  size(1200, 600);
  frameRate(60);
  background(bgColor);
  
  // Serial initialization
  
  gwSerial = new Serial(this, Serial.list()[0], 115200);
  
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
  
}



void draw() {
  background(200);

  float imageReduction = 0.15;
  
  int imgSizeWidth = int(imgFile.width*imageReduction);
  int imgSizeHeight = int(imgFile.height*imageReduction);
  
  image(imgFile, int(width-imgSizeWidth*1.5), int(height-imgSizeHeight*1.05), imgSizeWidth, imgSizeHeight);
  
  textSize(12);
  text(outputText,50,200);
  textAlign(LEFT,CENTER);
  fill(50);
  
  totalFileSize = (totalFileSize == 0)?1:totalFileSize;
  //float progValue = norm(currentFileSize, 0,totalFileSize);
  
  progress.setProgress(norm(currentFileSize, 0,totalFileSize));
  progressLbl.setLabel("Transfer Progress "+str(round(progress.getProgress()*100))+"%"+" File Size: "+str(float(totalFileSize/1024))+"kb or "+str(totalFileSize)+" bytes");
  if(imgRead) println("CurrentFileSize: "+str(currentFileSize)+" TotalFileSize: "+str(totalFileSize));
  
  if(cmdComplete){
    
    if(!imgRead){
    
     cmdBuf.trim();
     println("cmdBuf recv: " + cmdBuf.trim());
     String[] m = match(cmdBuf.trim(), "AV\\+(.*?);");
     
     if(m != null) {
       String[] tkn = splitTokens(m[1], ", ");
       
       if(match(tkn[0],"CS") != null) outputText = "Initialized";
       
       else if(match(tkn[0],"CTRANS") != null) {
         totalFileSize = int(tkn[1]);
         outputText = "Getting Image..." ;
         imgRead = true;
       }
       else if(match(tkn[0],"CTRANF")!=null){
         outputText = "Image transfer complete";
       }
       
       //outputText = m[1];
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
  String filename = sketchPath()+"/"+"IMAGE01.jpg";
  imgWriter = createOutput(filename);
  currentFileSize =0;
  totalFileSize=0;
  /*
  try{
    fstream = new FileOutputStream(filename);
    bstream = new BufferedOutputStream(fstream);
    dstream = new DataOutputStream(bstream);
  }
  catch(Exception e){
    println("Exception " + e);
    
  }
  */
  
}

void actionPerformed (GUIEvent e) {
  if (e.getSource() == getSnapshotBtn) {
    snapshotCmd();
  } 
}

void serialEvent(Serial s){

    if(cmdComplete) return;
    
    if(imgRead){
      
      if(currentFileSize < totalFileSize){
      
            try{
              imgWriter.write(s.read());
            }
            catch(IOException e){
              println("IOException");
            }
            
            
            currentFileSize++;
            //println(" Filesize: "+str(currentFileSize));  
         
            
       }
     else{
      

       println("File transfer complete. Current File size: "+str(currentFileSize)+" bytes");
       imgRead = false;
     
       try{
         imgWriter.flush();
         imgWriter.close();
       }
       catch(IOException e){
         println("Exception on close");
       }
       cmdComplete = true;
     
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