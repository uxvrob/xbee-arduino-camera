#include "Node.h"
#include "CameraNode.h"

#define CAM_RX_PIN 2
#define CAM_TX_PIN 3

CameraNode camNode = CameraNode(CAM_RX_PIN, CAM_TX_PIN);

void setup() {
  // put your setup code here, to run once:
  //Serial.begin(9600);
  //Serial.begin(57600);
  //Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  /*
  if(Serial.available()){
    Serial.write(Serial.read());
  }
  */

}
