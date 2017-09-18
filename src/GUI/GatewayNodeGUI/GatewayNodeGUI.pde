/*************************************************************** 
* Gateway Node GUI 
*
* @author Robbie Sharma robbie -at- rsconsulting.ca
* @date August 21, 2017
* @desc GUI for direct file transfer from GUI
* 
***************************************************************/


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
  
  int xpos_resetBtn = xpos_getSnapshotBtn-btnWidth-10;
  int ypos_resetBtn = ypos_getSnapshotBtn;
  
  int xpos_progressBar = int(xpos_getSnapshotBtn+btnWidth+10);
  int ypos_progressBar = int(ypos_getSnapshotBtn+btnHeight/4);
  
  int xpos_progressLbl = xpos_progressBar;
  int ypos_progressLbl = ypos_progressBar-20;
  
  int xpos_txtAConsole = 50;
  int ypos_txtAConsole = 50;
                 
                
  try{
    getSnapshotBtn = new IFButton ("Take Snapshot and Get", xpos_getSnapshotBtn, ypos_getSnapshotBtn, btnWidth, btnHeight);
    getSnapshotBtn.addActionListener(this);
    c.add (getSnapshotBtn);
    
    getRecentSnapBtn = new IFButton ("Get Recent Snapshot", xpos_getRecentSnapBtn, ypos_getRecentSnapBtn, btnWidth, btnHeight);
    getRecentSnapBtn.addActionListener(this);
    c.add (getRecentSnapBtn);
    
    resetBtn = new IFButton("RESET",xpos_resetBtn,ypos_resetBtn, btnWidth, btnHeight);
    resetBtn.addActionListener(this);
    c.add (resetBtn);
    
    progress = new IFProgressBar (xpos_progressBar, ypos_progressBar, int(width*0.4));
    c.add (progress);
    
    progressLbl = new IFLabel("Transfer Progress",xpos_progressLbl,ypos_progressLbl, 12);
    c.add (progressLbl);
    
  }
  catch(Exception e){
    println ("Unhandeled exception");
    exit();
  }
  
  defaultLook = new IFLookAndFeel(this, IFLookAndFeel.DEFAULT);
  c.setLookAndFeel(defaultLook);
  
  try{
    imgFile = loadImage(sketchPath() +"/default.jpg");
  }
  catch(Exception e){
    println ("Could not load main image... exiting...");
    exit();
  }
  
  txtAConsole = cp5.addTextarea("txt")
                  .setPosition(xpos_txtAConsole,ypos_txtAConsole)
                  .setSize(270,320)
                  .setFont(createFont("arial",12))
                  .setLineHeight(14)
                  .setColor(color(128))
                  .setColorBackground(color(255,100))
                  .setColorForeground(color(255,100))
                  .showScrollbar();
  
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
  
  resetBtn.setPosition(xpos_imgFile + imgSizeWidth + 10, ypos_imgFile);
  
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

    if((millis()-timer)>TIMEOUT){
      txtAConsole.setText(txtAConsole.getText()
                                   + "Gateway timeout on image transfer. Time: " + str(timer) + " ms\n"
                                   + "Received File size: "+str(currentFileSize)+" Total File Size: "+ str(totalFileSize)+"\n\n");
               
      
          progress.setProgress(norm(currentFileSize, 0,totalFileSize));
          progressLbl.setLabel("Transfer Timeout Occurred! "+str(round(progress.getProgress()*100))+"%"+" File Size: "+str(float(totalFileSize/1024))+"kb or "+str(totalFileSize)+" bytes");
    
        resetAll();

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
  
  
  
  processSerialResponse();
  
 
}




void serialEvent (Serial s){
  
    if(cmdComplete) return;
    
    if(!imgRead){
      char inChar = s.readChar();
      cmdBuf += str(inChar);
      
      if(inChar == ';' || inChar == '\n'){
          cmdComplete = true;   
      }
      
      if(cmdBuf.length() > 100){
        cmdBuf="";
        s.clear();
      }
    }
    else if(imgRead){
      
          try{
              int d_size = s.available();
              byte[] d = new byte[d_size];
              
              d = s.readBytes(d_size);
              
              imgWriter.write(d);
              currentFileSize += d_size;
              
              timer = millis(); // Timer reset
              
              d = null;

          }
          catch(IOException e){
            e.printStackTrace();
            txtAConsole.setText(txtAConsole.getText()
              + "Exception on serial read\n");
          }
      
    }
    
}