#ifndef CAMERA_NODE_H
#define CAMERA_NODE_H

#include <Node.h>
#include <SoftwareSerial.h>
#include <Adafruit_VC0706.h>

#define CAM_RX_PIN 2
#define CAM_TX_PIN 3


typedef struct {
  char szName[15];
  uint16_t uSize;
  uint16_t uPackets;
  uint16_t uPacketIndex;
} image_file_t;


class CameraNode : public Node{
	
	public:
	
	image_file_t _ift;
	
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
