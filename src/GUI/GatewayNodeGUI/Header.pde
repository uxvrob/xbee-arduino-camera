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

/*****************************************************************
* Header.pde 
*
* @author Robbie Sharma <robbie@rsconsulting.ca>
* @date August 21, 2017
* @desc Global variables and configuration for GatewayNodeGUI.pde
* 
******************************************************************/

import controlP5.*;
import interfascia.*;
import processing.serial.*;
import java.io.File;

// Serial parameters
/********************************************************************
 * Determine active serial port in Serial.list()[x] for port refernece
 * by uncommenting printArray block in setup() method
 *******************************************************************/

// Camera Node - COM3
// Gateway Node - COM4

final String SER_PORT   = "COM4";       //Serial.list()[2];
final int SER_BAUD_RATE = 57600;      //This should match the Serial baud rate in the GatewayNode firmware
final int BUF_SIZE      = 32;              

// Window parameters

final int bgColor = 200;

// Button Parameters

final int btnWidth = 100;
final int btnHeight = 30;

// Image pixel size in GUI

final int imgSizeWidth =  640;
final int imgSizeHeight = 480;

// Image streaming timeout

final int TIMEOUT = 3000;  // in milliseconds (ms)

// GUI Controls - Interfascia
GUIController c;

IFLookAndFeel defaultLook;

IFButton getSnapshotBtn;
IFButton getRecentSnapBtn;
IFButton resetBtn;

IFProgressBar progress;
IFLabel progressLbl;

// GUI Controls - ControlP5
ControlP5 cp5;
Textarea txtAConsole;

// Command Parser/Buffer

String cmdBuf = "";
boolean cmdComplete = false;
boolean imgRead;
boolean sdSave;

// Snapshot Image File Parameters

PImage imgFile;
OutputStream imgWriter;
String recvImgFileName="";

int totalFileSize = 0;
int currentFileSize = 0;
int timer = 0;
int errCount = 0;
char response;

boolean resetFlag = false;

byte[] buffer;

Serial gwSerial;

/******************************************************
* Exception Codes
*******************************************************/
final char ku8MBSuccess               = 0x00;
final char ku8MBIllegalFunction       = 0x01;
final char ku8MBIllegalDataAddress     = 0x02;
final char ku8MBIllegalDataValue     = 0x03;
final char ku8MBSlaveDeviceFailure     = 0x04;
final char ku8MBInvalidSlaveID       = 0xE0;
final char ku8MBInvalidFunction       = 0xE1;
final char ku8MBResponseTimedOut     = 0xE2;
final char ku8MBInvalidCRC         = 0xE3;