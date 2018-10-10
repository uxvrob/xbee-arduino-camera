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
* Utility.pde 
*
* @author Robbie Sharma <robbie@rsconsulting.ca>
* @date August 21, 2017
* @desc Helper functions for GatewayNodeGUI.pde
* 
***************************************************************/

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