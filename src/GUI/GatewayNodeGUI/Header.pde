import controlP5.*;
import interfascia.*;
import processing.serial.*;

// Serial parameters
/********************************************************************
 * Determine active serial port in Serial.list()[x] for port refernece
 * by uncommenting printArray block in setup() method
 *******************************************************************/

// Camera Node - COM3
// Gateway Node - COM4

final String SER_PORT   = "COM3";       //Serial.list()[2];
final int SER_BAUD_RATE = 57600;      //This should match the Serial baud rate in the GatewayNode firmware
final int BUF_SIZE      = 64;              

// Window parameters

final int bgColor = 200;

// Button Parameters

final int btnWidth = 100;
final int btnHeight = 30;

// Image pixel size in GUI

final int imgSizeWidth =  640;
final int imgSizeHeight = 480;

// Image streaming timeout

final int TIMEOUT = 30000;  // in milliseconds (ms)

// GUI Controls - Interfascia
GUIController c;
IFLookAndFeel defaultLook;
IFButton getSnapshotBtn;
IFButton getRecentSnapBtn;
IFProgressBar progress;
IFLabel progressLbl;

// GUI Controls - ControlP5
ControlP5 cp5;
Textarea txtAConsole;

// Command Parser/Buffer

String cmdBuf = "";
boolean cmdComplete = false;
boolean imgRead;

// Snapshot Image File Parameters

PImage imgFile;
OutputStream imgWriter;
String recvImgFileName="";

int totalFileSize = 0;
int currentFileSize = 0;
int timer = 0;
int errCount = 0;
char response;

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