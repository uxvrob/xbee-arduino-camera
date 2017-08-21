import processing.serial.*;

final int BUF_SIZE = 64;

int bgcolor;           // Background color
int fgcolor;           // Fill color
Serial myPort;                       // The serial port
byte[] buffer = new byte[BUF_SIZE+2];    // Where we'll put what we receive
int serialCount = 0;                 // A count of how many bytes we receive
boolean firstContact = false;        // Whether we've heard from the microcontroller
String line;

void setup(){
  
  size(256, 256);  // Stage size
  noStroke();      // No border on the next thing drawn
  
  PrintWriter imgFile;
  int jpgLen =0;
  
  printArray(Serial.list());
  
  
  myPort = new Serial(this, Serial.list()[0], 115200);
  myPort.buffer(BUF_SIZE);
  
  
  // Sleep for a bit
  delay(200);
  
  try {
    //Send serial command AV+CGETS command
    myPort.write("AV+CGETS\n");
    delay(300);
    // Listen for response - get file size
    
    line = myPort.readStringUntil(int("\n"));
    
    println("Received string: "+line);
  }catch (Exception e){
    e.printStackTrace();
    myPort.clear();
    myPort.stop();
    exit();
  }
  
  if(line.indexOf("AV+CTRANS") != -1)
    jpgLen = int(line.substring(line.indexOf(",")+1));
  
  println("File size: "+str(jpgLen));
  
  // Check and create image file
   
   String filename = "IMAGE00.jpg";
   imgFile = createWriter(filename);
  
  /*
  for (int i = 0; i < 1000; i++) {
    filename[5] = '0' + i/10;
    filename[6] = '0' + i%10;
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
    imgFile.print(myPort.readBytes(bytesToRead));
    
    myPort.write("&");
    
    jpgLen -= bytesToRead;

  }
  
  println("Read bytes, flushing file and closing");
  imgFile.flush();
  imgFile.close();
  
  if(myPort.readChar() == '+'){
    line = myPort.readString();
    
    if(line.equals("AV+CTRANF"))
      println("Successfully received image");
    
  }
  
  
  
  //
}

void draw() {
  background(bgcolor);
  fill(fgcolor);
  // Draw the shape
}