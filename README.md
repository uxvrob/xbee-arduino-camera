# Avalon
<img src="sch/PrototypeView.jpg" 
alt="Avalon Prototype" width="50%" height="50%" border="10" />

Camera xBee-Arduino wireless transceiver that communicates with a xBee-Arduino wireless gateway interfaced by USB-Serial to a Processing GUI to control camera snapshots and image transmission.

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See deployment for notes on how to deploy the project on a live system.

### Prerequisites

#### Hardware
* 2 x Arduino UNO (https://store.arduino.cc/usa/arduino-uno-rev3)
* 2 x Arduino Wireless SD Shield - RETIRED (https://store.arduino.cc/usa/arduino-wirelss-sd-shield)
* 2 x xBee S2C (Datasheet: https://www.digi.com/resources/documentation/digidocs/pdfs/90002002.pdf)
* 1 x Adafruit TTL Serial JPEG Camera (https://www.adafruit.com/product/397)
* 2 x 9V battery connector
* Jumper Wire
* 1 x USB Cable

#### Software

* Arduino IDE (https://www.arduino.cc/en/Main/Software)
* XCTU (https://www.digi.com/products/xbee-rf-solutions/xctu-software/xctu)
* Processing IDE (https://processing.org/download/)

### Building

#### Camera module

<img src="sch/CameraModule-Connection-Wiring.jpg" 
alt="Camera Module Connection Wiring" width="50%" height="50%" border="10" />

Build this module with this tutorial: https://learn.adafruit.com/ttl-serial-camera/

#### Camera Node
##### Top View
<img src="sch/CameraNode-TopView.jpg" 
alt="CameraNode Top View" width="50%" height="50%" border="10" />

##### Bottom View
<img src="sch/CameraNode-Shield-BottomView.jpg" 
alt="CameraNode Shield Wiring" width="50%" height="50%" border="10" />

#### Gateway Node
##### Top View
<img src="sch/GatewayNode-TopView.jpg" 
alt="GatewayNode TopView" width="50%" height="50%" border="10" />

##### Bottom View
<img src="sch/GatewayNode-Shield-BottomView.jpg" 
alt="GatewayNode Shield Wiring" width="50%" height="50%" border="10" />


### Installing

#### xBee Configuration 

1. Ensure the “Serial Select” switch is at USB on both Arduino Wireless SD shields.  
  a. When the “Serial Select” switch is set to USB, the xBee will communicate through the USB-FTDI chip via its RX, TX lines.  When it is set to “micro”. The RX, TX lines of the xBee are tied to the TX, RX lines of the Arduino Uno respectively.  This is called “Serial Hijacking”.  
  
2. Open the Arduino IDE and go to File->Basics->Bare Minimum.  

3. Upload the code to GatewayNode and CameraNode Arduinos.  
  a. This ensures that there is no Arduino interference for when the xBee’s are configured.  
  
4. Go to https://www.digi.com/resources/documentation/digidocs/90001526/default.htm  

5. Follow the guide to download, install and configure the XBEE modules using XCTU.    
  a. DigiMesh 2.4 TH should be the installed firmware  
  b. XBEE_A should be GatewayNode  
  c. XBEE_B should be CameraNode  


| Parameter |	Description |	GatewayNode |	CameraNode |
| ---------| ------------| ------------| ------------| 
| ID | Network ID |	D161	| D161 | 
| DH |	Destination Address HIGH |	0013A200 |	0013A200 | 
| DL |	Destination Address LOW	| SL of CameraNode | SL of GatewayNode | 
| NI | Node Identifier | GatewayNode	CameraNode | 
| PL | TX Power Level | 0 |	0 | 
| BD |	Baud Rate |	115200 | 115200 | 
| Arduino_SN | 	xBee Serial | 	55635303838351319170 | 	5563530383835161B131 | 

#### Basic Communication Test

1. Ensure the “Serial Select” switch is at USB on both Arduino Wireless SD shields.  

2. Open “gatewayNode-basic-comm-test.ino”  

3. Upload to GatewayNode on the CORRECT COM port  
  a. Ensure the correct COM port is selected if both Arduino’s are plugged in via USB  

4. Open “cameraNode-basic-comm-test.ino”  

5. Upload to CameraNode device  
  a. Ensure the correct COM port is selected if both Arduino’s are plugged in via USB  
  
6. Switch “Serial Select” to “Micro” on BOTH Arduino Wireless SD Shields  

7. Open Serial monitor on “CameraNode” COM Port (Check Arduino SN)  
  a. Output should be:  
      i. Hello World  
    
8. Open Serial monitor on “GatewayNode” COM port (Check Arduino SN)  
  a. Output should be:  
      i. RX: H  
     ii. RX: e  
     iii. RX: l  
     iv. RX: l  
     v. RX: o  
    vi. RX:  
    vii. RX: W  
    viii.	RX: o  
    ix.	RX: r  
    x. RX: l  
    xi. RX: d  
    xii. RX:   

9. This confirms a successful serial connection between both xBee devices



## Running the tests


## Deployment

Add additional notes about how to deploy this on a live system

## Built With

* [Arduino IDE](https://www.arduino.cc/) - The Arduino IDE used to program Arduino UNO
* [xBee](https://www.digi.com/) - Wireless Transceiver 
* [Processing](https://processing.org/) - Used to create GUI

## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details on our code of conduct, and the process for submitting pull requests to us.

## Authors

* **Robbie Sharma** - (http://github.com/uxvrob)

## License

This project is licensed under the GNU GPL License - see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments

* Thanks to Sensorum for the project!
