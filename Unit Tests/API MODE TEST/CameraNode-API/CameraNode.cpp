/*------------------------------------------------------------------------
  This file is part of the Smartifi - Avalon project

 <INSERT LICENSE SCHEME>
 
 
  ------------------------------------------------------------------------*/

#include "Node.h"
#include "CameraNode.h"


CameraNode::CameraNode(int cam_rx_pin, int cam_tx_pin):
_camConn(cam_rx_pin, cam_tx_pin),
_cam(_camConn)
{
  
}

void CameraNode::takePicture(){

  
}


