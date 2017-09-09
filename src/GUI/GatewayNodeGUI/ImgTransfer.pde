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
    gwSerial.buffer(5);
    sendGatewayCmd("+");
    delay(100);

    while((totalFileSize-currentFileSize) > 0 && (millis()-timer)<TIMEOUT){
      
      char response = ku8MBSuccess;
      int bufferIndex = 0;
      int u32StartTime = millis();
      int bytesToRead = min(BUF_SIZE, (totalFileSize-currentFileSize))+5;
      
      while(bytesToRead > 0 && (response == ku8MBSuccess)){
           
            if(gwSerial.available()>0){
                
                buffer[bufferIndex++] = (byte)gwSerial.read();
                              
                if(buffer[0] == 0){  // discard any 0 bytes;
                    bufferIndex--;   
                    continue;
                }
                
                bytesToRead--;
                
            }
            else{
              
            }
            
            
            if(bufferIndex == 2){
              if(buffer[0] != 0x76) response = ku8MBIllegalFunction; 
              if(int(buffer[1]) != (bytesToRead-3)) {
                println("Expected: ",bytesToRead-3,"Received: ",buffer[1]);
                response = ku8MBIllegalDataValue;
                break;
              }
            }
            
            if (millis() > (u32StartTime + 2000)) {
              response = ku8MBResponseTimedOut;
              break;
            }
            
          
       }
       
       if(response == ku8MBSuccess && bufferIndex >= 2){
             /*     
             int u16CRC = 0xFFFF;
              
              for(int i=0; i < int(buffer[1]); i++){
                print(i,":",hex(buffer[i+2]), " ");
                u16CRC = _crc16_update(u16CRC, buffer[i+2]);
              }
              
              if ((u16CRC != word(buffer[bufferIndex - 3], buffer[bufferIndex - 2]))) {
                  response = ku8MBInvalidCRC;
                  println("invalid crc calculated:",hex(u16CRC),"received high: ",hex(buffer[bufferIndex - 3]), "low: ", hex(buffer[bufferIndex - 2]));
              }
              else println("valid CRC");
              
              */
              
              if((int(buffer[1]) != int(buffer[bufferIndex-3])) && (int(buffer[1]) != int(buffer[bufferIndex-2]))){
                  println("invalid crc calculated:",int(buffer[1]),"received high: ",int(buffer[bufferIndex - 3]), "low: ", int(buffer[bufferIndex - 2]));
              
                response = ku8MBInvalidCRC;
              }            
       }
            
       if(response == ku8MBSuccess){
    
                
              // Read stream data and output to jpg file
              try{
                
                
                for(int i=0; i < int(buffer[1]); i++){
                  imgWriter.write(buffer[i+2]);
                  /*
                  if(i%5 == 0)
                    print("[",i, " : ", hex(buffer[i+2]),"]");
                    */
                }
               
                
                currentFileSize+=(int(buffer[1]));
                
  
              }
              catch(IOException e){
                e.printStackTrace();
                txtAConsole.setText(txtAConsole.getText()
                  + "Exception on serial read\n");
              }
              
              // Output a response every 128 bytes
              if(((currentFileSize % 128) == 0)){
                 println("Current Filesize: "+str(currentFileSize)+" Buffer size: "+str(min(64, (totalFileSize-currentFileSize))) + "\n");
                 timer = millis(); // Timer reset
              }
                
         
       }
       
       printExceptionCode(response);
       
       switch (response){   
          case ku8MBSuccess:
            gwSerial.clear();
            delay(10);
            sendGatewayCmd("+");
            break;
          case ku8MBIllegalFunction:
          case ku8MBIllegalDataAddress:     
          case ku8MBIllegalDataValue:   
          case ku8MBSlaveDeviceFailure: 
          case ku8MBInvalidSlaveID:       
          case ku8MBInvalidFunction:      
          case ku8MBResponseTimedOut:    
          case ku8MBInvalidCRC:
            gwSerial.clear();
            delay(10);
            sendGatewayCmd("-");
            break;
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
    
    imgRead = false;
    return;

}