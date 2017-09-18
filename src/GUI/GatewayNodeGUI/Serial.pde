void processSerialResponse(){
  /**********************************************************
  ******************** SERIAL PROCESSING *********************
  **********************************************************/
  
  if(cmdComplete){
    
    if(!imgRead){
    
     cmdBuf.trim();
     println("cmdBuf recv: " + cmdBuf.trim());
     String[] m = match(cmdBuf.trim(), "AV\\+(.*?);");
     
     if(m != null) {
       String[] tkn = splitTokens(m[1], ", ");
       
       if(match(tkn[0],"CS") != null){ 
         
         
         
           txtAConsole.setText(txtAConsole.getText()
                        + "Connected to Camera Node\n");      // AV+CS,0x01;
                        
           if(match(tkn[1],"0x01") != null){
             
             txtAConsole.setText(txtAConsole.getText()
                        + "Camera is active and ready to take snaps!\n\n"); 
             
           }
           else{
             
             txtAConsole.setText(txtAConsole.getText()
                        + "Camera is having problems... is it plugged in properly?\n"
                        + "Press RESET on Camera Node after fixing\n\n");
           }
          
       
       }else if(match(tkn[0],"CTRANS") != null) {                           // AV+CTRANS,<img_file_size_in_bytes>;  Start image transfer
       
         totalFileSize = int(tkn[1]);
         println("MATCH: TOTALFILESIZE",totalFileSize);
         txtAConsole.setText(txtAConsole.getText() + 
                             "Getting Image of size: " + totalFileSize + "...\n");
                             
         // Switch to reading image byte stream in Serial Event handler and start timeout timer
         println("Starting img transfer thread...");
         
         createImageFile(); 
         currentFileSize=0;
         imgRead = true;
         timer = millis();
         
         
       }else if(match(tkn[0],"SDSAVE") != null) {                           // AV+SDSAVE,<packet_index>,<packets>;  Status of SD Card save;
         println("PacketIndex: " + tkn[1] + " Packets: " + tkn[2]);
       
       }else if(match(tkn[0],"DEBUG") != null){
         
         if(match(tkn[1],"TAKESNAP_SD") != null){
           txtAConsole.setText(txtAConsole.getText()
                               + "Snapshot taken!\n"
                               + "Arduino filename: "+tkn[2]+" Filesize: "+tkn[3]+" bytes\n");
           
         }
         
       }
       else if(match(tkn[0],"ERROR") != null){
         
         txtAConsole.setText(txtAConsole.getText()
                               + "Error: "
                               + tkn[2]+"\n");
                               
        resetAll();
       
       }
      
       println("Match: "+m[1]);
     }
    }
    
    cmdComplete = false;
    cmdBuf="";
  } 
  
}