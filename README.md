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

A step by step series of examples that tell you how to get a development env running

Say what the step will be

```
Give the example
```

And repeat

```
until finished
```

End with an example of getting some data out of the system or using it for a little demo

## Running the tests

Explain how to run the automated tests for this system

### Break down into end to end tests

Explain what these tests test and why

```
Give an example
```

### And coding style tests

Explain what these tests test and why

```
Give an example
```

## Deployment

Add additional notes about how to deploy this on a live system

## Built With

* [Arduino IDE](https://www.arduino.cc/) - The Arduino IDE used to program Arduino UNO
* [xBee](https://www.digi.com/) - Wireless Transceiver 
* [Processing](https://processing.org/) - Used to create GUI

## Contributing

Please read [CONTRIBUTING.md](https://gist.github.com/PurpleBooth/b24679402957c63ec426) for details on our code of conduct, and the process for submitting pull requests to us.

## Authors

* **Robbie Sharma** - (http://github.com/uxvrob)

## License

This project is licensed under the GNU GPL License - see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments

* Thanks to Sensorum for the project!
