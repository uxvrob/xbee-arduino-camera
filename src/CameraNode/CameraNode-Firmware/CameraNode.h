/***************************************************************
* CameraNode.h
*
* @author Robbie Sharma robbie -at- rsconsulting.ca
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
