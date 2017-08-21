import processing.serial.*;

final int BUF_SIZE = 64;

int bgcolor=0;           // Background color
int fgcolor;           // Fill color


Serial gwSerial;                       // The serial port
String cmdBuf = "";
boolean cmdComplete = false;

PrintWriter imgFile;

boolean time = false;

int timer(int timerLength) {
  int remainingTime = timerLength-millis();
  
  println(str(remainingTime));
 
  if(remainingTime/1000>0){
    int actualTime = (remainingTime/1000);
    return actualTime;
   }
  else {
    time = false;
    return 0;     
  }
}




void getSnapshot(){
  int jpgLen =0;
  cmdComplete = true;
  
  println("Getting snapshot");
  
  cmdBuf = "";
  
  while(gwSerial.available()>0){
   char inChar = gwSerial.readChar();
   cmdBuf += str(inChar);
   
   
   
   println("RX:"+str(inChar));
  }
  
  println("cmdBuf: "+cmdBuf.trim());
  
  gwSerial.write("AV+CGETS");
  gwSerial.write(10);
  cmdBuf = "";
  
  
  
  String[] s = split(cmdBuf,",");
  
  try{
    jpgLen = int(s[1].substring(0,s[1].indexOf(";")));
  }
  catch(Exception e){
    println("jpglen Exception: "+e);
    gwSerial.clear();
    gwSerial.stop();
    return;
  }
  
  println("File size: "+str(jpgLen));
  
  // Check and create image file
   
   String filename = "IMAGE01.jpg";
   imgFile = createWriter(sketchPath()+"/"+filename);
  
  /*
  for (int i = 0; i < 1000; i++) {
    filename.charAt(5) = '0' + i/10;
    filename.charAt(6) = '0' + i%10;
    // create if does not exist, do not open existing, write, sync after write
    if (! SD.exists(filename)) {
      break;
    }
   }
  */
  
  // Loop and gather data
  
  while(jpgLen > 0){
  
    int bytesToRead = min(BUF_SIZE, jpgLen);
    
    println("jpgLen: "+str(jpgLen));
    imgFile.print(gwSerial.readBytes(bytesToRead));
    
    jpgLen -= bytesToRead;
    
  }
  
  println("Read bytes, flushing file and closing");
  imgFile.flush();
  imgFile.close();
  
  if(gwSerial.readChar() == '+'){
    String line = gwSerial.readString();
    
    if(line.trim().equals("AV+CTRANF"))
      println("Successfully received image");
    
  }
  
  
  
}

void setup(){
  
  size(800, 500);  // Stage size
  noStroke();      // No border on the next thing drawn
  
 

  
  printArray(Serial.list());
  
  
  gwSerial = new Serial(this, Serial.list()[0], 115200);
  delay(500);
}

void draw() {
  background(bgcolor);
  textSize(12);
  //fill(fgcolor);
  // Draw the shape
  text(cmdBuf,100,100);
  
  while(gwSerial.available() > 0){
    
    char inChar = gwSerial.readChar();
    
    cmdBuf += str(inChar);
    
    
    
  }
  
  /*
  if(cmdComplete){
    
      
      if(cmdBuf != null){
      
        println("CMD BUF AFTER TRIM: "+cmdBuf.trim());
        
        if(cmdBuf.trim().equals("AV+CS,0x01;")){
          println("Camera on successful");
          getSnapshot();
        }
      }
      else 
        println ("cmdBuf is null");
      cmdBuf = "";
      cmdComplete = false;
  }
  */
  

}

void serialEvent(Serial ser){
  
  while(ser.available()>0){
    char inChar = ser.readChar();
    

    cmdBuf += str(inChar);
      
    if(inChar == ';'){
      if(cmdBuf != null){
        
        cmdComplete = true;
      }
      else {
        cmdBuf = "";
        cmdComplete = false;
      }
    
    }
  }
}