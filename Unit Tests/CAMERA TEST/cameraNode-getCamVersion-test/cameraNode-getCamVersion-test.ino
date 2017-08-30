// Camera node on COM 5
// Serial: 5563530383835161B131
// Arduino Uno Pin out: http://www.pighixxx.com/test/wp-content/uploads/2017/05/uno.png

//#include <Adafruit_VC0706.h>
#include <SoftwareSerial.h>

const byte CAM_RX_PIN = 3; //A4(18) -> white wire -> RX_B_Camera
const byte CAM_TX_PIN = 2; //A5(19) -> gray wire -> TX_B_Camera

// SoftwareSerial(rxpin,txpin)
SoftwareSerial camSerial = SoftwareSerial(CAM_RX_PIN, CAM_TX_PIN);
//SoftwareSerial camSerial = SoftwareSerial(18, 19);
//Adafruit_VC0706 cam = Adafruit_VC0706(&camSerial);

void setup() {
    // Start the serial port
    Serial.begin(38400);
    camSerial.begin(38400);
    //Serial.println("VC0706 Camera snapshot test");

    // Try to locate the camera
    /*
    if (cam.begin(38400)) {
      Serial.println("Camera Found:");
    } else {
      Serial.println("No camera found?");
      return;
    }
    */
}

void loop() {

    while (Serial.available())
      camSerial.write(Serial.read());
    while (camSerial.available())
      Serial.write(camSerial.read());
  /*
  // Print out the camera version information (optional)
  char *reply = cam.getVersion();
  if (reply == 0) {
    Serial.print("Failed to get version");
  } else {
    Serial.println("-----------------");
    Serial.print(reply);
    Serial.println("-----------------");
  }
  
  delay(1000);
  */
  /*
  if(cameraconnection.available() > 0){
    Serial.print(cameraconnection.read(), HEX);
    Serial.print("\t");
  }
    */
}


