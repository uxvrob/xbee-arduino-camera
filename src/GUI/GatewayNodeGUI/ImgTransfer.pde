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
      println("Response Timed Out");
      break;
    case ku8MBIllegalDataAddress: 
    case ku8MBSlaveDeviceFailure:
    case ku8MBInvalidSlaveID:       
    case ku8MBInvalidFunction:      
       println("Invalid response");
       break;
    
  }
  
}

void processImgInput(){    
    timer = millis();
    createImageFile(); 
    // LOOP UNTIL ENTIRE FILE IS READ

    while((totalFileSize-currentFileSize) > 0 && (millis()-timer)<TIMEOUT){
                
              // Read stream data and output to jpg file
              if(gwSerial.available()>0){
                try{
                  
                  imgWriter.write((byte)gwSerial.read());
                  currentFileSize++;
                  
    
                }
                catch(IOException e){
                  e.printStackTrace();
                  txtAConsole.setText(txtAConsole.getText()
                    + "Exception on serial read\n");
                }
              }
              
              // Output a response every 128 bytes
              if(((currentFileSize % 32) == 0) && currentFileSize > 500){
                 println("Current Filesize: "+str(currentFileSize)+" Buffer size: "+str(min(64, (totalFileSize-currentFileSize))) + "\n");
                 timer = millis(); // Timer reset
              }
                
         
     }
    
    
    try{
         imgWriter.flush();
         imgWriter.close();
     }
     catch(IOException e){
       e.printStackTrace();
       txtAConsole.setText(txtAConsole.getText()+
                           "Exception generated on file close...\n");
                            
    }
    if(currentFileSize == totalFileSize){
      println("Image receipt successful!!");
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
    }
    imgRead = false;
    return;

}