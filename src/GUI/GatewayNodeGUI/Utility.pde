
int word(byte high, byte low) {
  int ret_val = low;
  ret_val += (high << 8);
  return ret_val;
}

int _crc16_update(int crc, byte a) {

  crc ^= a;
  for (int i = 0; i < 8; ++i)
  {
    if ((crc & 1)==1)
      crc = (crc >> 1) ^ 0xA001;
    else
      crc = (crc >> 1);
  }
  return crc;
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
  currentFileSize =0;
  
}