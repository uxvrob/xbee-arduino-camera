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
    case ku8MBIllegalDataAddress: 
    case ku8MBSlaveDeviceFailure:
    case ku8MBInvalidSlaveID:       
    case ku8MBInvalidFunction:      
       println("Invalid response");
       break;
    
  }
  
}

void processImgInput(Serial s){    
  
    // LOOP UNTIL ENTIRE FILE IS READ
    while((totalFileSize-currentFileSize) > 0){
      
      char response = ku8MBSuccess;
      
      while(bytesToRead > 0 && (response != ku8MBSuccess)){
        
            if(s.available()>0){
                
                buffer[bufferIndex++] = byte(s.read());
                if(buffer[0] == 0){  // discard any 0 bytes;
                    bufferIndex--;   
                    continue;
                }
                
                bytesToRead--;
                
                println("Bytes left: ", bytesToRead);
  
            }else{
               delay(10); 
            }
            
            if(bufferIndex == 2){
              if(buffer[0] != 0x76) response = ku8MBIllegalFunction; 
              if(int(buffer[1]) != (bytesToRead-3)) {
                println("Expected: ",bytesToRead-3,"Received: ",buffer[1]);
                response = ku8MBIllegalDataValue;
                break;
              }
            }
            
            if (millis() > (u32StartTime + 1000)) {
              response = ku8MBResponseTimedOut;
              println("Timed out");
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
                cmdComplete = true;
                return;
              }            
       }
            
       if((totalFileSize-currentFileSize) > 0  && bytesToRead <= 0 && response == ku8MBSuccess){
    
                
                // Read stream data and output to jpg file
                println ("writing image data currentFileSize: ", currentFileSize);
                try{
                  
                  for(int i=0; i < int(buffer[1]); i++){
                    imgWriter.write(buffer[i+2]);
                    print(i, " : ", hex(buffer[i+2]));
                  }
                  println(" ");
                  
                  currentFileSize+=(int(buffer[1]));
                  timer = millis(); // Timer reset
    
                }
                catch(IOException e){
                  e.printStackTrace();
                  txtAConsole.setText(txtAConsole.getText()
                    + "Exception on serial read\n");
                }
                
                // Output a response every 128 bytes
                if(((currentFileSize % 128) == 0) && currentFileSize > 20000){
                          println("Current Filesize: "+str(currentFileSize)+" Buffer size: "+str(min(64, (totalFileSize-currentFileSize))) + "\n");
                  
                }
                
                cmdComplete = true;
       }
   
    }
    return;

}