//Xbee transmit file in packet mode.
// 
// Transmits a file to a designated xbee radio.

// NB: The Xbee radios must be configured for packet mode (API) 
//     To do this, modify the Xbee radio settings using XCFU and 
//     set the "AP" parameter to "2"

// Create the xbee object
#include <XBee.h>
#include <SPI.h>
#include <SD.h>

/*
This example is for Series 2 XBee
 Sends a file
*/

// create the XBee object
XBee xbee = XBee();

/*****************************************************************************************/
/*****************************************************************************************/
// Set the following parameters for this transfer
int debug = 1;  // Set debug = 0 to remove debug statements. 
//String filename = "test.txt";         // Name of the file to open
String filename = "image20.jpg";         // Name of the file to open
uint32_t receiver_msb = 0x0013a200;   // Set most sig bit for the address of receiving xbee
uint32_t receiver_lsb = 0x41671e21;   // LSB for the Green LED
//uint32_t receiver_lsb = 0x41671f5d; // LSB for the Red LED
const uint8_t max_payload_size = 90; // Max size in Bytes of the data sent in a packet.
const int chipSelect = 4;             // Tells the system where the SD card is.
//const int buttonPin = 8;            // the digital input of the pushbutton pin
const int statusPin = 6;              // digital output for the LED that indicates success
const int errorPin = LED_BUILTIN;     // digital output for the LED that indicates failure
//int buttonState = 0;         // variable for reading the pushbutton status
//int last_buttonState = 0;    // Holds the last state 

// Global Vars
uint8_t file_header[] = {'S','e','n','s','o','r','u','m',' ','H','e','a','d','e','r',' ','v','1'};
uint8_t dummy_payload[] = {0}; // Declare dummy payload in order to construct the ZBTxRequest object.
XBeeAddress64 addr64 = XBeeAddress64(receiver_msb, receiver_lsb);         // Set the address of the receiving xbee
ZBTxRequest zbTx = ZBTxRequest(addr64, dummy_payload, sizeof(dummy_payload)); // Declare the tx var
ZBTxStatusResponse txStatus = ZBTxStatusResponse();      
AtCommandRequest atRequest = AtCommandRequest();
AtCommandResponse atResponse = AtCommandResponse();
ZBRxResponse rx = ZBRxResponse();
ModemStatusResponse msr = ModemStatusResponse();
uint8_t myMAC[8];  // MAC address of the xbee connected to this device.
uint8_t receiverMAC[8]; // MAC address in bytes of the receiver (transcribed from receiver_msb and receiver_lsb above)
uint8_t shCmd[] = {'S','H'}; // Command to send to the xbee to get the high part of the MAC
uint8_t slCmd[] = {'S','L'}; // Command to send to the xbee to get the low part of the MAC
String err_msg;
uint8_t fid = 2;
/*****************************************************************************************/
/*****************************************************************************************/
// Print out a string to Serial
void debug_out(String s) {
  //if (debug) Serial.println(s);
}


/*****************************************************************************************/
/*****************************************************************************************/
// Simple function to flash an LED some number of times at some rate.
//   Very useful for debugging when no access to serial monitor to PC
//   b/c serial bus is used to talk to xbee radio. 
void flashLed(int pin, int times, int wait) {

  for (int i = 0; i < times; i++) {
    digitalWrite(pin, HIGH);
    delay(wait);
    digitalWrite(pin, LOW);

    if (i + 1 < times) {
      delay(wait);
    }
  }
}

/*****************************************************************************************/
/*****************************************************************************************/

// Sends AT command to the Xbee to get information such as the MAC address
int sendAtCommand() {
  //Serial.println("Sending command to the XBee");

  // send the command
  xbee.send(atRequest);

  // wait up to 5 seconds for the status response
  if (xbee.readPacket(5000)) {
    // got a response!

    // should be an AT command response
    if (xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE) {
      xbee.getResponse().getAtCommandResponse(atResponse);

      if (atResponse.isOk()) {
        if (atResponse.getValueLength() > 0) {
        }
        return 0;
      } 
      else {
        //err_msg += "Command return error code: " + String(atResponse.getStatus(),HEX) + "\n";
        // Serial.print("Command return error code: ");
        //Serial.println(atResponse.getStatus(), HEX);
        return -1;
      }
    } else {
      //err_msg += "Expected AT response but got: ";
      //err_msg += String(xbee.getResponse().getApiId(),HEX) + "\n";
      // Serial.print("Expected AT response but got ");
      //Serial.print(xbee.getResponse().getApiId(), HEX);
      return -2;
    }   
  } else {
    // at command failed
    if (xbee.getResponse().isError()) {
      //err_msg += "Error reading packet. Error code: ";
      //err_msg += String(xbee.getResponse().getErrorCode()) + "\n";
      //Serial.print("Error reading packet.  Error code: ");  
      //Serial.println(xbee.getResponse().getErrorCode());
      return -3;
    } 
    else {
      // Serial.print("No response from radio");  
      // Not an error, just no response.
      return 1;
    }
  }
  return 1;
}

/*****************************************************************************************/
/*****************************************************************************************/
// Loads the global "myMAC[]" array with the MAC address of the xbee attached to this device.
//  Loads the receiverMAC[] array with the MAC address of the xbee to which info is to be sent
// Returns 0 if successful and a negative integer if unsuccessful.
int load_MACs() {
  // set command to SH to get high part of MAC address
  atRequest.setCommand(shCmd); 
  while (sendAtCommand()) {Serial.println("getting mac address : Retry command ATSH");};
  //int offset = atResponse.getValueLength();
  for (int i=0;i<4;i++){
    myMAC[i] = atResponse.getValue()[i];
  }
  // set command to SH to get low part of MAC address
  atRequest.setCommand(slCmd);  
  while (sendAtCommand()) {Serial.println("getting mac address : Retry command ATSL");};
  for (int i=0;i<4;i++){
    myMAC[i+4] = atResponse.getValue()[i];
  }

  // Load receiverMAC[8] by unrolling receiver_msb and receiver_lsb;
  receiverMAC[0] = receiver_msb >> 24;
  receiverMAC[1] = receiver_msb >> 16;
  receiverMAC[2] = receiver_msb >> 8;
  receiverMAC[3] = receiver_msb;
  receiverMAC[4] = receiver_lsb >> 24;
  receiverMAC[5] = receiver_lsb >> 16;
  receiverMAC[6] = receiver_lsb >> 8;
  receiverMAC[7] = receiver_lsb;

  return 0;
}

/*****************************************************************************************/
/*****************************************************************************************/
// Given an array of bytes and it's size, constructs a packet and sends it.
int send_packet(uint8_t payload[], int payload_size){
    // Construct the ZBTxRequest object and send it.
    zbTx = ZBTxRequest(addr64, payload, payload_size);
    fid = 2;
    //fid += 2;
    //if (fid == 0) fid = 2;
    zbTx.setFrameId(fid);
    //Serial.println("send_packet: FrameId = "+ String(fid));
    xbee.send(zbTx);    
    // After sending a tx request, we expect a status response
    // wait up to half second for the status response
    if (xbee.readPacket(500)) {
      // got a response!
      // should be a znet tx status              
      if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
        xbee.getResponse().getZBTxStatusResponse(txStatus);
        // get the delivery status, the fifth byte
        if (txStatus.getDeliveryStatus() == SUCCESS) {
          // success.  time to celebrate
          //flashLed(statusPin, 1, 100);
          digitalWrite(errorPin,LOW);
          //Serial.println("received ACK, returning...");
          return 0;
        } else {
          // the remote XBee did not receive our packet. is it powered on?
          flashLed(errorPin, 5, 100);
          digitalWrite(errorPin,HIGH);
          return 1;
        }
      }
    } else if (xbee.getResponse().isError()) {
      //nss.print("Error reading packet.  Error code: ");  
      //nss.println(xbee.getResponse().getErrorCode());
      flashLed(errorPin, 5, 200);
      digitalWrite(errorPin,HIGH);
      return 2;
   
    } else {
      // local XBee did not provide a timely TX Status Response -- should not happen
      flashLed(errorPin, 10, 500);
      digitalWrite(errorPin,HIGH);
      return 3;
    }  

    //Serial.println("existing send_packet()");    
    return 0;
} // send_packet()

/***********************************************************************************/
/***********************************************************************************/
// Read packet from xbee in API mode. The contents of the packet are loaded into the 
//  rx global data structure. Returns 0 if read of a packet is received and acknowledged
//  by the sender. Otherwise returns not 0 
int read_packet() {
    
  xbee.readPacket();
  if (xbee.getResponse().isAvailable()) {
    // got something
    if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
      // got a zb rx packet
      // now fill our zb rx class
      xbee.getResponse().getZBRxResponse(rx);
            
      if (rx.getOption() == ZB_PACKET_ACKNOWLEDGED) {
        // the sender got an ACK
        //if (debug) Serial.println ("readPacket: got a response and an ACK - Success");
        return 0;
      } 
      else {
          // we got it (obviously) but sender didn't get an ACK
          //if (debug) Serial.println ("readPacket: got a response but no ACK");
          return 1;
      }
    } 
    else if (xbee.getResponse().getApiId() == MODEM_STATUS_RESPONSE) {
      xbee.getResponse().getModemStatusResponse(msr);
      // the local XBee sends this response on certain events, like association/dissociation
      if (msr.getStatus() == ASSOCIATED) {
        // yay this is great.  flash led
        //if (debug) Serial.println ("readPacket: response but it's association");
        return 1;
      } else if (msr.getStatus() == DISASSOCIATED) {
        // this is awful.. flash led to show our discontent
        //if (debug) Serial.println ("readPacket: response but it's disassociation (this is bad)");
        return -1;
      } else {
        // another status
        //flashLed(statusLed, 5, 50);
        //if (debug) Serial.println ("readPacket: got a response but it's not something we know");
        return 1;
      }
    } 
    else {
      // not something we were expecting
      //if (debug) Serial.println ("readPacket: unexpected response");
      return -1;
    }
  } // end if (xbee.getResponse().isAvailable())
  else if (xbee.getResponse().isError()) {
    //nss.print("Error reading packet.  Error code: ");  
    
    //if (debug) Serial.println ("readPacket: xbee responded with an error. Error code:");
    //if (debug) Serial.println (xbee.getResponse().getErrorCode());
    return -1;
  }
  else {
    //if (debug) Serial.println ("readPacket: no response available");
    return 1; // No response available.
  }
} // End read_packet()

/***********************************************************************************/
/***********************************************************************************/
// Handshake. Processes a confirmation packet from the device to which a packet had
// just been sent.  Returns 0 if successful and not 0 if unsuccessful
int handshake(uint32_t checksum, uint32_t id) {
  // Receives a payload in a specified format that contains info that can be used
  //  to verify the receipt of a packet by the device that was just sent a packet 
  //  by this device.
  // The format is as follows:
  // 18 Bytes: file_header --> Indicates this is the first packet of a file transfer
  // 8 Bytes: MAC address  --> MAC address of the xbee attached to this device
  // 4 Bytes: id           --> The id of the packet (number) that was sent
  // 4 Bytes: checksum     --> the checksum of the packet that was sent.
  //   
  // Note that the payload is of fixed size equal to 34 bytes.
  uint8_t *payload;
  int cycle = 0;
  int max_cycles = 2000;
  while(read_packet()) {
    // Did not receive a packet, try again
    //debug_out("waiting to receive handshake for packet " + String(id));
    delay(1);   
    if (max_cycles < cycle++) return -1;
  }

  // First set a pointer to the payload of the packet for convenience
  payload = rx.getData();
  uint8_t payload_size = rx.getDataLength(); // size of payload.

  // First 18 bytes contains the header and must match the header defined here as a global data structure
  int pos=0;
  // Verify the first 8 bytes match.
  for (int i=0;i<sizeof(file_header);i++) {
    if (payload[pos++] != file_header[i]) {
      //if (debug) Serial.println("file header doesn't match: " + String(id));
      return -2;
    }
  }  

  // Extract the MAC address of the device that sent this handshake (the receiver device of the original packet)
  // Next 8 bytes are the MAC of the receiver and must match the address stored in "receiverMAC"
  for (int i=0;i<sizeof(receiverMAC);i++) {
    if (payload[pos++] != receiverMAC[i]) {
      //if (debug) Serial.println("receiverMAC doesn't match MAC from packet: " + String(id));
      return -3;
    }
  }

  // Extract the id from the packet and make sure it matches that passed to this function
  uint32_t tmp32[4];
  for (int i=0;i<4;i++) { tmp32[i] = payload[pos++];}
  uint32_t local_id = ( (tmp32[0] << 24) | (tmp32[1] << 16) | (tmp32[2] << 8) | tmp32[3]);
  if (id != local_id) {
    // trap for Errors
    //if (debug) Serial.println("handshake: id != local_id");
    return -4;
  }
  // Extract the checksum from the packet and make sure it matches that passed to this function
  for (int i=0;i<4;i++) { tmp32[i] = payload[pos++];}
  uint32_t local_checksum = ( (tmp32[0] << 24) | (tmp32[1] << 16) | (tmp32[2] << 8) | tmp32[3]);
  if (checksum != local_checksum) {
    // trap for Errors
    //if (debug) Serial.println("handshake: checksum != local_checksum");
    return -5;
  }
  return 0;
} // handshake()


/***********************************************************************************/
/***********************************************************************************/
// Takes a filename and address of a receiving xbee and transmits the file to the 
// xbee. The receiver must be running complementary code on its end. Also, it is
// assumed that a global xbee object has been declared.
int transmit_file_to_xbee(String fname, uint32_t msb, uint32_t lsb) {
  // Local Vars
  uint8_t payload[max_payload_size]; // Holds the payload itself.
  uint32_t checksum;
  uint32_t id;

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    //Serial.println("Card failed, or not present");
    // don't do anything more:
    flashLed(errorPin,2,100);
    digitalWrite(errorPin,HIGH);
    return -1;
  }  
  File myFile = SD.open(fname,FILE_READ);;
  if (myFile) {
    unsigned long filesize = myFile.size();
    // Payload size is the net number of bytes that can be used to transmit parts of the file
    //  It is equal to the max_payload size minus the number of bytes required for header info
    int payload_size = max_payload_size - sizeof(myMAC); // Assumes only header info required is the MAC
    if (payload_size <= 0) {
      // Error state because there's no room for data to be sent!
      // Throw Error!!
      return -2;
    }
    uint8_t last_packet_size = filesize % payload_size; 
    uint32_t num_packets = filesize / payload_size;

    if (last_packet_size > 0) num_packets++;

    // Create first packet, which contains info on the rest of the packets 
    //   (e.g. size and number) and how to reconstruct the file.
    // The payload of the first packet has the following format:
    // 18 Bytes: file_header --> Indicates this is the first packet of a file transfer
    // 8 Bytes: MAC address  --> MAC address of the xbee attached to this device
    // 4 Bytes: num_packets  --> number of packets following this one
    // 4 Bytes: filesize     --> size in bytes of the file.
    // 1 Byte:  filename_length --> number of bytes describing the filename
    // N Bytes: Filename     --> the filename being transferred, N is filename_length
                              
    uint8_t filename_length = filename.length()+1;
    
    uint8_t filename_uint8[filename_length];
    filename.getBytes(filename_uint8,filename_length);
    uint8_t first_payload_size = sizeof(file_header) + 8 + 4 + 4 +1 + filename_length;
    uint8_t first_payload[first_payload_size];

    // First object is the File Header defined above
    uint8_t pos = 0;
    for (int i=0; i<sizeof(file_header); i++) {
      first_payload[pos++] = file_header[i];
    }    
    // Second object is the MAC address
    for (int i=0; i<sizeof(myMAC); i++) {
      first_payload[pos++] = myMAC[i];
    }
    // Third object is the number of packets
    first_payload[pos++] = num_packets >> 24;
    first_payload[pos++] = num_packets >> 16;
    first_payload[pos++] = num_packets >> 8;
    first_payload[pos++] = num_packets;
    // Fourth object is the raw file size
    first_payload[pos++] = filesize >> 24;
    first_payload[pos++] = filesize >> 16;
    first_payload[pos++] = filesize >> 8;
    first_payload[pos++] = filesize;
    // Fifth object is the length of the filename
    first_payload[pos++] = filename_length;
    // Sixth object is the filename itself.    
    for (int i=0; i<filename_length; i++) {
      first_payload[pos++] = filename_uint8[i];
    }

    if (pos != first_payload_size) { 
      // Trap for error. 
      return -3;
    }

    // Calculate a simple checksum on the bytes and use this as proof of receipt by the receiver.
    checksum = 0;
    //if (debug) Serial.println("printout of first payload:");
    for (int i=0;i<sizeof(first_payload);i++) {
      checksum += first_payload[i];
    }
    // Send first packet.
    int err = send_packet(first_payload,first_payload_size);
    if (err) {
     //Trap for errors...
     return -4;
    }

    // Wait for a reply from the receiver, confirming receipt of the packet.
    id = -1; // Unique id of the first packet is a uint32_t set to -1 (i.e. all 1's). 
    //if (debug) Serial.println("Waiting for first handshake");
    int handshake_result = handshake(checksum,id);
    if (handshake_result < 0) {
      //Trap for errors
      return -5;
    }

    // Send the remaining packets
    //myFile.seek(0);
    bool packet_sent = false;
    int cycle = 0;
    int max_cycles = 10;
    for (int i=0;i<num_packets; i++) {
      //Serial.println("\n ************ \n sending file packet i = " + String(i));
      // Load the first 8 bytes with the MAC address
      pos = 0;
      for (int j=0;j<sizeof(myMAC);j++) { payload[pos++] = myMAC[j];};
      // Load file data up to the size of the payload or whatever is remaining
      //pos = 8;//  sizeof(myMAC);
      //Serial.println("pos = "+ String(pos));
      //Serial.println("max_payload_size = " + String(max_payload_size));
      //Serial.println("myFile.available() = " + String(myFile.available()));
      while( myFile.available() && (pos < max_payload_size)) {
        payload[pos++] = myFile.read();
      }

      checksum = 0;
      for (int j=0;j<pos;j++) {
        checksum += payload[j];
      }

      Serial.println("\n processing packet: " + String(i)); 

      //delay(3000);

      int send_packet_result;
      packet_sent = false;
      cycle = 0;
      max_cycles = 10;
      while(!packet_sent && (cycle < max_cycles)) {
        Serial.print("\n sending a packet for cycle = ");
        Serial.print( String(cycle));
        //err = send_packet(payload,max_payload_size);
        send_packet_result = send_packet(payload,pos);
        if (send_packet_result) {
          // Trap for errors
          cycle++;
        }
        else {  // No error, proceed with handshake
          
          // Wait for a reply from the receiver, confirming receipt of the packet.
          // If handshake() returns 0, that means the packet was sent successfully.
          //   otherwise, handshake() != 0, means it was not sent.
          id = i;
          handshake_result = handshake(checksum,id);
        
          if (!handshake_result) {
            packet_sent = true;
          }
          else {
            Serial.print(" handshake failed. err code = ");
            Serial.print(String(handshake_result));
            cycle++;
          }
        } //  if (send_packet_result)   
      } // while (!packet_sent && cycle<max_cycle)
      if (!packet_sent) {
        // Packet not sent for some reason, figure it out
        Serial.println("\n ERROR: Packet not sent");
        Serial.println("  send_packet result = " + String(send_packet_result));
        Serial.println("  handshake result = " + String (handshake_result));
        return -6;
      }
      else {
        Serial.println("  packet_sent!!");
      }
    } // for (num_packets)

    myFile.close();
  } else { // opening myFile failed, throw error.
     flashLed(errorPin, 3, 100);
     digitalWrite(errorPin, HIGH);
     return -8;
  }
  return 0;
} // transmit_file_to_xbee()

/*****************************************************************************************/
/*****************************************************************************************/
void setup() {
  // initialize serial communication:
  Serial.begin(9600); 
  xbee.begin(Serial);
     
  // initialize the various input and output pins:
  //pinMode(buttonPin, INPUT); 
  pinMode(statusPin, OUTPUT);    
  pinMode(errorPin, OUTPUT);    

  // Get the MAC address of this device - need to include this in all packets.
  load_MACs();  // call to load myMAC with the MAC of this xbee.

  // Call function to transfer contents of file to xbee.
  //debug_out("about to call transmit_file_to_xbee");
  int err = transmit_file_to_xbee(filename, receiver_msb, receiver_lsb);
  Serial.println("transmit_file_to_xbee = " + String(err));
  
}

/*****************************************************************************************/
/*****************************************************************************************/
void loop(){
} // end loop 


