#ifndef CAMERA_NODE_H
#define CAMERA_NODE_H

#include <Node.h>
#include <SoftwareSerial.h>
#include <Adafruit_VC0706.h>

#define CAM_RX_PIN 2
#define CAM_TX_PIN 3
#define IMG_BUF_SIZE 64         // Byte batch of image to send via Serial

typedef struct {
  char szName[15];
  uint16_t uSize;
} image_file_t;


class CameraNode : public Node{
	
	public:

    CameraNode(int,int);


    void begin();
    void takeSnapshotSaveToSD(image_file_t*);
    void sendSnapshotFile(char*);


	private:
    Node* _nd;
	  SoftwareSerial* _camConn;
    Adafruit_VC0706* _cam;
    bool _debugOn;

};


#endif
