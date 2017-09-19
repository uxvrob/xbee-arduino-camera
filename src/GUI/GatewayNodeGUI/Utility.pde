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
  
  if(e.getSource() == resetBtn){
    

    
    resetFlag = true;
    resetAll();
    
  }
}

void sendGatewayCmd(String cmd){
  
  gwSerial.write(cmd);
  gwSerial.write(10); 

}

void resetAll(){
  
      txtAConsole.setText(txtAConsole.getText() 
                            + "Resetting...\n");

    timer=0;
    imgRead = false;
    cmdBuf = "";
    cmdComplete=false;

   saveImageFile();
   
  
  deleteImageFile();

 
   gwSerial.stop();
   gwSerial = null;
   

   try{
      gwSerial = new Serial(this, SER_PORT, SER_BAUD_RATE);
   }
   catch(Exception e){
     e.printStackTrace();
     txtAConsole.setText(txtAConsole.getText()+
                         "Exception re-start serial...\n");
  }

   
   txtAConsole.setText(txtAConsole.getText()+
                           "Soft reset complete... press reset button on the Arduinos...\n");
                           
   resetFlag = false;
  
}