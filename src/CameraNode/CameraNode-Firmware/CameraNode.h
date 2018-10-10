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
* CameraNode.h
*
* @author Robbie Sharma <robbie@rsconsulting.ca>
* @date September 16, 2017
* @desc Implement to take images, save to SD, and transmit via xBee-Serial
 bridge.  Later, API commands will be used.
* 
***************************************************************/

#ifndef CAMERA_NODE_H
#define CAMERA_NODE_H

#include <Node.h>
#include <SoftwareSerial.h>
#include <Adafruit_VC0706.h>

typedef struct {
  char szName[15];
  uint16_t uSize;
  uint16_t uPackets;
  uint16_t uPacketIndex;
} image_file_t;


class CameraNode : public Node{
	
	public:
	
	image_file_t _ift;
	bool transferActive;
	bool SDSaveActive;
	
    CameraNode(int,int);

    void begin();
    void takeSnapshotSaveToSD();
    void sendSnapshotFile(char*);
	
	protected:
	
	static void _zbRxCb(ZBRxResponse&, uintptr_t);

	private:
	
    Node* _nd;
	SoftwareSerial* _camConn;
    Adafruit_VC0706* _cam;
	
};


#endif
