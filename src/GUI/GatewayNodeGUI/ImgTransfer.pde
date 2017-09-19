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

void deleteImageFile(){
  
     File f = new File(sketchPath()+"/"+recvImgFileName); 
   if(f.exists()){
     f.delete();
     println("Deleted image file");
   }
   else
     println("could not delete image file");
  
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