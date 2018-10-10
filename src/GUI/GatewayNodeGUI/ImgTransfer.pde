/********************************************************************************
 * 
    Copyright (c) 2017-2018 Robbie Sharma. All rights reserved

    This file is part of xbee-arduino-camera.
    <https://github.com/uxvrob/xbee-arduino-camera>

    xbee-arduino-camera is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    xbee-arduino-camera is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with xbee-arduino-camera.  If not, see <https://www.gnu.org/licenses/>.
    
********************************************************************************/

/*************************************************************** 
* ImgTransfer.pde 
*
* @author Robbie Sharma <robbie@rsconsulting.ca>
* @date August 21, 2017
* @desc Image processing functions for GatewayNodeGUI.pde
* 
***************************************************************/

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