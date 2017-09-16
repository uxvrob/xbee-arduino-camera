void actionPerformed (GUIEvent e) {
  
  if (e.getSource() == getSnapshotBtn) {
    
    if(!imgRead){
      txtAConsole.setText(txtAConsole.getText() 
                          + "Waiting for image data...\n"
                          + "Grab a coffee... this will take awhile..\n");
      
      
      sendGatewayCmd("AV+SGETS");
    }
    else{
          txtAConsole.setText(txtAConsole.getText() 
                          + "Error: cmd not sent... transfer in progress...\n");
    }
    


  } 
  
  if(e.getSource() == getRecentSnapBtn){

    if(!imgRead){
      txtAConsole.setText(txtAConsole.getText() 
                            + "Getting most recent image stored...\n");
      
      sendGatewayCmd("AV+CSEND");
    }
    else{
          txtAConsole.setText(txtAConsole.getText() 
                          + "Error: cmd not sent... transfer in progress...\n");
    }
     
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
  
}

void resetAll(){

    timer=0;
    imgRead = false;
    cmdBuf = "";
    cmdComplete=false;
    
    
    

    saveImageFile();
  
   gwSerial.clear();
   gwSerial.stop();
   gwSerial = null;
   delay(100);
   while(gwSerial != null){
     try{
        gwSerial = new Serial(this, SER_PORT, SER_BAUD_RATE);
     }
     catch(Exception e){
       e.printStackTrace();
       txtAConsole.setText(txtAConsole.getText()+
                           "Exception re-start serial...\n");
    }
    
   }
  
}