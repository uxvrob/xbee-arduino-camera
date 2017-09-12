#ifndef CAMERA_NODE_H
#define CAMERA_NODE_H

#include <Node.h>
#include <SoftwareSerial.h>

#define CAM_RX_PIN 2
#define CAM_TX_PIN 3

class CameraNode : public Node {
	
	public:

    CameraNode(int,int);
    void takePicture();

 
	
	private:
	  SoftwareSerial* _camConn;
    Adafruit_VC0706 _cam;

};


#endif
