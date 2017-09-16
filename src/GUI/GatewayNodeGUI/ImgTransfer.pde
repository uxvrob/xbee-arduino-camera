void processImgInput(){    

    createImageFile(); 
    // LOOP UNTIL ENTIRE FILE IS READ
    
    timer = millis();
    currentFileSize =0;

    while((totalFileSize-currentFileSize) > 0 && (millis()-timer)<TIMEOUT){
                
              
        // Output a response every 128 bytes
        if(((currentFileSize % 128) == 0)){
           println("Current Filesize: "+str(currentFileSize)+" Buffer size: "+str(min(BUF_SIZE, (totalFileSize-currentFileSize))) + "\n");
           timer = millis(); // Timer reset
        }
                
         
    }
    
    if(currentFileSize < totalFileSize){
      
      int bytesAvail = gwSerial.available();
      
      if(bytesAvail >0){
         try{
         imgWriter.write(gwSerial.readBytes()); 
         }
         catch(Exception e){
           
           e.printStackTrace();
           txtAConsole.setText(txtAConsole.getText()+
                         "Exception generated serial read in ProcessImgInput...\n");
           
         }
      }
        
    }
    
    
   
    if(currentFileSize == totalFileSize){
      println("Image receipt successful!!");
      saveImageFile();
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