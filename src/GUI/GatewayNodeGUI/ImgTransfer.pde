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
  
  println("Image file created: " + recvImgFileName);
}

void saveImageFile(){
  
  try{
    if(imgWriter != null){
       imgWriter.flush();
       imgWriter.close();
    }
   }
   catch(IOException e){
     e.printStackTrace();
     txtAConsole.setText(txtAConsole.getText()+
                         "Exception generated on file close...\n");
  }
  
  println("Saving image file... " + recvImgFileName);
  
}

void closeImageTransfer(){
  
        saveImageFile();
        
        println("Image receipt successful!!");

        txtAConsole.setText(txtAConsole.getText()
                                 +"Transfer complete. File: " + recvImgFileName + "\n");
         try{
           imgFile = loadImage(sketchPath() + "/" + recvImgFileName);
           println( "New image width x height: "+str(imgFile.width)+" x " + str(imgFile.height));
         }
         catch(Exception e){
             e.printStackTrace();
             txtAConsole.setText(txtAConsole.getText()
                           +"Could not load image... currentFileSize: "+currentFileSize + "\n\n");
    
             imgFile = loadImage(sketchPath() +"/default.jpg");
         }
  
}

void processImgInput(){    

    createImageFile(); 
    // LOOP UNTIL ENTIRE FILE IS READ
    
    imgRead = true;
    cmdComplete = false;
    
    timer = millis();
    currentFileSize =0;

    while((currentFileSize<=totalFileSize)){

        // Output a response every 128 bytes
        if(((currentFileSize % 128) == 0)){
           println("Current Filesize: "+str(currentFileSize)+"\n");
           timer = millis();
        }
        
        if(resetFlag ) return;
        
        if((millis()-timer)>TIMEOUT) {
          
          txtAConsole.setText(txtAConsole.getText()
                                 +"Transfer timeout!\n");
          
          resetAll();
          return;
        }
         
    }
    
    saveImageFile();
   
    if(currentFileSize == totalFileSize){
      println("Image receipt successful!!");

      txtAConsole.setText(txtAConsole.getText()
                                 +"Transfer complete. File: " + recvImgFileName + "\n");
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
    else{
      
      println("Image size not good.  currentFileSize: ");
      txtAConsole.setText(txtAConsole.getText()
                                 +"Transfer problems.  Missing " + (totalFileSize - currentFileSize) + " bytes...\n");
      
    }

    imgRead = false;
    return;

}