//Xbee receive in packet mode.
//
// NB: The Xbee radios must be configured for packet mode (API) 
//     To do this, modify the Xbee radio settings using XCFU and 
//     set the "AP" parameter to "2"

#include <XBee.h>
#include <SPI.h>
#include <SD.h>

/*
This example is for Series 2 XBee 
Receives a file sent by ZB RX packets from a device running xbee_API_transmit_file_v1.
*/

XBee xbee = XBee();
XBeeResponse response = XBeeResponse();

// Global Vars
//uint8_t *payload;
// Unique file header that ensures the packet format will match between sender and receiver
uint8_t file_header[] = {'S','e','n','s','o','r','u','m',' ','H','e','a','d','e','r',' ','v','1'};
int file_header_size = 18;
int debug = 1;
int loop_err;
uint8_t senderMAC[8];  // MAC address of the xbee sending info to this device.
uint8_t myMAC[8];      // MAC address of the xbee connected to this device.
uint32_t num_packets;
uint32_t file_size;
String filename;
uint8_t fid = 1;
uint8_t shCmd[] = {'S','H'}; // Command to send to the xbee to get the high part of the MAC
uint8_t slCmd[] = {'S','L'}; // Command to send to the xbee to get the low part of the MAC
// Create reusable objects related to the xbee calls.
uint8_t dummy_payload[] = {0};  // Need a dummy payload for the ZBTxRequest constructor
uint32_t dummy_addr = 0;        // Need a dummy address for addr64 constructor
XBeeAddress64 addr64 = XBeeAddress64(dummy_addr, dummy_addr);  // Need a dummy addr64 object for the ZBTxRequest constructor
ZBTxRequest tx = ZBTxRequest(addr64, dummy_payload, sizeof(dummy_payload)); // Declare the tx var
ZBTxStatusResponse txStatus = ZBTxStatusResponse();      
AtCommandRequest atRequest = AtCommandRequest();
AtCommandResponse atResponse = AtCommandResponse();
ZBRxResponse rx = ZBRxResponse();
ModemStatusResponse msr = ModemStatusResponse();

File myFile;


// I/O pins
const int LED_OUTPUT = 6; // Set pin 6 to be the output 
const int errorPin = LED_BUILTIN;

/***********************************************************************************/
/***********************************************************************************/
/***********************************************************************************/
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

/***********************************************************************************/
// Sends AT command to the Xbee to get information such as the MAC address
// Must have already set the desired command in the atRequest object!
// Returns 0, if times out with no response or if response is invalid or in error
// Returns 1 if get response and the response is OK
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
        return 1;  // Response is ok, so return 1. 
      } 
      else {
        if (debug) {
          //Serial.print("SendAtCommand: atResponse.getStatus() = ");
          //Serial.println(atResponse.getStatus(), HEX);
        }
      }
    } else {
      if (debug) {
        //Serial.print("sendAtCommand: Expected AT response but got ");
        //Serial.print(xbee.getResponse().getApiId(), HEX);
      }
    }   
  } 
  else { // Waited for backet but at command failed
    if (xbee.getResponse().isError()) {
      if (debug) {
        //Serial.print("SendAtCommand: Error reading packet.  Error code: ");  
        //Serial.println(xbee.getResponse().getErrorCode());
      }
      //return 1;
    } 
    else {
      if (debug) {
        //Serial.print("No response from radio");    
      }
      //return 1;
    }
  }
  return 0;
}

/*****************************************************************************************/
// Loads the global "myMAC[]" array with the MAC address of the xbee attached to this device.
void get_MAC() {
  // set command to SH to get high part of MAC address
  atRequest.setCommand(shCmd); 
  while (!sendAtCommand()) {Serial.println("getting mac address : Retry command ATSH");};
  //int offset = atResponse.getValueLength();
  for (int i=0;i<4;i++){
    myMAC[i] = atResponse.getValue()[i];
  }
  // set command to SL to get low part of MAC address
  atRequest.setCommand(slCmd);  
  while (!sendAtCommand()) {Serial.println("getting mac address : Retry command ATSL");};
  for (int i=0;i<4;i++){
    myMAC[i+4] = atResponse.getValue()[i];
  }
}

/*****************************************************************************************/
// Given an address and an array of bytes and it's size, constructs a packet and sends it.
int send_packet(uint32_t msb, uint32_t lsb, uint8_t payload[], int payload_size){

  addr64.setMsb(msb);
  addr64.setLsb(lsb);
  tx = ZBTxRequest(addr64, payload, payload_size);
  tx.setFrameId(fid);
  xbee.send(tx);    

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
        return 0;
      } else {
        // the remote XBee did not receive our packet. is it powered on?
        flashLed(errorPin, 5, 100);
        digitalWrite(errorPin,HIGH);
        return 1;
      }
    }
    else { // xbee.getResponse().getApiId() != ZB_TX_STATUS_RESPONSE
      Serial.println("send_packet xbee response = " + String(xbee.getResponse().getApiId()));
      return -33;
    }
  } 
  else if (xbee.getResponse().isError()) {
    //Serial.print("Error reading packet.  Error code: ");  
    //Serial.println(xbee.getResponse().getErrorCode());
    flashLed(errorPin, 5, 200);
    digitalWrite(errorPin,HIGH);
    return 2;
  } 
  else {
    // local XBee did not provide a timely TX Status Response -- should not happen
    flashLed(errorPin, 10, 500);
    digitalWrite(errorPin,HIGH);
    //Serial.println("send_packet:: local xbee did not provide a timely TX status response");
    return 3;
  }  
  return -1;
} // send_packet()


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
          return -1;
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
        return -2;
      } else {
        // another status
        //flashLed(statusLed, 5, 50);
        //if (debug) Serial.println ("readPacket: got a response but it's not something we know");
        return -3;
      }
    } 
    else {
      // not something we were expecting
      //if (debug) Serial.println ("readPacket: unexpected response");
      return -4;
    }
  } // end if (xbee.getResponse().isAvailable())
  else if (xbee.getResponse().isError()) {
    //nss.print("Error reading packet.  Error code: ");  
    
    //if (debug) Serial.println ("readPacket: xbee responded with an error. Error code:");
    //if (debug) Serial.println (xbee.getResponse().getErrorCode());
    return -5;
  }
  else {
    //if (debug) Serial.println ("readPacket: no response available");
    return -6; // No response available.
  }
} // End read_packet()

/***********************************************************************************/
// Assumes rx contains the first packet of a collection of packets from which a file
//  can be reconstructed. This first packet must be in a specific format:
// 18 Bytes: file_header --> Indicates this is the first packet of a file transfer
// 8 Bytes: MAC address  --> MAC address of the xbee attached to this device
// 4 Bytes: num_packets  --> number of packets following this one
// 4 Bytes: filesize     --> size in bytes of the file.
// 1 Byte:  filename_length --> number of bytes describing the filename
// N Bytes: Filename     --> the filename being transferred, N is filename_length
int process_first_packet() {
  // Read first packet
  while(read_packet()) {
    // Did not receive a packet, try again
    delay(10);   
  }
  
  //if (debug) Serial.println("process_first_packet()");
  
  // First set a pointer to the payload of the packet for convenience
  uint8_t *payload;
  payload = rx.getData();
  if (debug) {
    for (int i=0;i<18;i++) {
      //Serial.println("payload[" + String(i) + "] = " + char(payload[i]));
    }
  }
  uint8_t payload_size = rx.getDataLength(); // size of payload.
  //if (debug) Serial.println("rx.getDataLength() = " + String(payload_size));
  // File header must match that from the sender. It's how one knows this is a first packet.
  int pos=0;
  // Verify the header from the packet matches the one here.
  for (int i=0;i<sizeof(file_header);i++) {
    if (payload[pos++] != file_header[i]) {
      //if (debug) Serial.println("headers do not match");
      return 1;
    }
  }

  // Header matches, next read in the MAC of the sender
  //   Store the MAC in the senderMAC[8]
  for (int i=0;i<sizeof(senderMAC);i++) { senderMAC[i] = payload[pos++]; }

  //if (debug) {
  //  for (int i=0;i<sizeof(senderMAC);i++) {
  //    Serial.println("senderMAC[i] = " + String(senderMAC[i],HEX));  
  //  }
  //}

  uint32_t tmp32[4]; // Needed to convert set of four bytes into a uint32_t
  // Extract the num_packets.
  for (int i=0;i<4;i++) { tmp32[i] = payload[pos++];}
  num_packets = ( (tmp32[0] << 24) | (tmp32[1] << 16) | (tmp32[2] << 8) | tmp32[3]);

  if (debug) Serial.println("num_packets = " + String(num_packets));

  // Extract the file size:
  for (int i=0;i<4;i++) { tmp32[i] = payload[pos++];}
  file_size = ( (tmp32[0] << 24) | (tmp32[1] << 16) | (tmp32[2] << 8) | tmp32[3]);

  //if (debug) Serial.println("file_size = " + String(file_size));

  // Extract the length of the filename
  uint8_t filename_length = payload[pos++];

  //if (debug) Serial.println("filename_length = " + String(filename_length));

  // Extract the filename
  for (int i=0;i<filename_length;i++) {filename += char(payload[pos++]);}

  //if (debug) Serial.println("filename = " + filename);

  /*-------------------------------------------------------------------------------*/
  // Finally, send packet back to sender verifying receipt of this first packet.

  // The handshake needs the msb and lsb of the sender, a unique id and a checksum of this payload
  
  // The most sig bit of the MAC address of the sender is constructed from the first four bytes
  for (int i=0;i<4;i++) { tmp32[i] = senderMAC[i];}
  uint32_t msb = ( (tmp32[0] << 24) | (tmp32[1] << 16) | (tmp32[2] << 8) | tmp32[3]);
  
  for (int i=0;i<4;i++) { tmp32[i] = senderMAC[i+4];}
  uint32_t lsb = ( (tmp32[0] << 24) | (tmp32[1] << 16) | (tmp32[2] << 8) | tmp32[3]);

  // The unique id for the first packet is the high bit (for unsigned int, this can be set by -1);
  uint32_t unique_id = -1;

  // Finally, calculate the checksum of this first payload.
  uint32_t checksum = 0;
  for (int i=0;i<payload_size;i++) { checksum += payload[i]; }
  //if (debug) Serial.println("first packet checksum = " + String(checksum));

  int err = handshake(msb,lsb,unique_id,checksum);
  if (err != 0) {
    // Trap for errors
    return 1;
  }
  return 0;
} // process_first_packet()

/***********************************************************************************/
// Assumes the first packet has been read and the following global variables have
// the correct values: senderMAC, num_packets, filesize, filename.
// Also assumes that the sender has transmitted packet id.
int process_packet(uint32_t id){
  //if (debug) Serial.println("process_packet() for packet # = " + String(id));
  // Read packet
  int max_wait_cycles = 10000;
  int cycle = 0;
  int read_result = read_packet();
  while(read_result) {
    // Did not receive a packet, try again
    //if (debug) {
      //if (read_result > 0) Serial.println("waiting to receive packet " + String(id) + " result = " + String(read_result));
    //}
    delay(5);   
    if (max_wait_cycles < ++cycle) {
      //if (debug) Serial.println(" Exceeded max wait cycles to precess next packet. exiting...");
      return -1;
    }
    read_result = read_packet();
  }

  //if (debug) Serial.println("  read_packet returned for packet " + String(id));

  // First set a pointer to the payload of the packet for convenience
  uint8_t *payload;
  payload = rx.getData();
  //if (debug) {
  //  for (int i=0;i<8;i++) {
  //    //Serial.println("payload[i] = " + String(payload[i],HEX));
  //  }
  //}
  uint8_t payload_size = rx.getDataLength(); // size of payload.
  //if (debug) Serial.println("rx.getDataLength() = " + String(payload_size));

  
  //if (debug) {
  //  Serial.println("printing out entire packet: ");
  //  for (int i=0;i<payload_size;i++) {
  //    Serial.println("payload[" + String(i) + "] = " + String(char(payload[i])));
  //  }
  //}
  
  // First 8 bytes are the MAC of the sender and must match the address stored in "senderMAC"
  int pos=0;
  // Verify the first 8 bytes match.
  for (int i=0;i<sizeof(senderMAC);i++) {
    if (payload[pos++] != senderMAC[i]) {
      //if (debug) Serial.println("senderMAC doesn't match MAC from packet: " + String(id));
      return 1;
    }
  }

  // Read remaining bytes into temp string for debug
  String payload_string;
  for (int i=8;i<payload_size;i++) {
    payload_string += char(payload[i]);
  }
  myFile.print(payload_string);
  
  //Serial.println("Payload for packet " + String(id) + ":");
  //Serial.println(payload_string);
  


  /*-------------------------------------------------------------------------------*/
  // Finally, send handshake packet back to sender verifying receipt of this first packet.

  // The handshake needs the msb and lsb of the sender, a unique id (the packet number), and 
  //  a checksum of this payload

  uint32_t tmp32[4]; // Needed to convert set of four bytes into a uint32_t
  
  // The most sig bit of the MAC address of the sender is constructed from the first four bytes
  for (int i=0;i<4;i++) { tmp32[i] = senderMAC[i];}
  uint32_t msb = ( (tmp32[0] << 24) | (tmp32[1] << 16) | (tmp32[2] << 8) | tmp32[3]);
  
  for (int i=0;i<4;i++) { tmp32[i] = senderMAC[i+4];}
  uint32_t lsb = ( (tmp32[0] << 24) | (tmp32[1] << 16) | (tmp32[2] << 8) | tmp32[3]);

  // Finally, calculate the checksum of this first payload.
  uint32_t checksum = 0;
  for (int i=0;i<payload_size;i++) { checksum += payload[i]; }

  Serial.print("\n processing packet " + String(id));
  //Serial.println("calculated checksum calling handshake");

  int handshake_result = handshake(msb,lsb,id,checksum);
  cycle = 0;
  int max_send_cycles = 5;
  while (handshake_result) {
    // Trap for errors
    Serial.println("process_packet handshake err = " + String(handshake_result));
    if (max_send_cycles < cycle++) {return 222;}
    handshake_result = handshake(msb,lsb,id,checksum);
  }
  return 0;
} // process_packet()

/***********************************************************************************/
// Sends a packet back to the sender with the checksum of the last packet. The sender
// will wait until receiving this handshake before sending the next packet.
// Assumes that the checksum has been calculated.
int handshake(uint32_t msb, uint32_t lsb, uint32_t id, uint32_t checksum) {

  // Construct a payload in a specified format that contains info that can be used
  //  to verify the receipt of a packet by this device to the device that sent it.
  // The format is as follows:
  // 18 Bytes: file_header --> Indicates this is the first packet of a file transfer
  // 8 Bytes: MAC address  --> MAC address of the xbee attached to this device
  // 4 Bytes: i_packet     --> The id of this packet (number)
  // 4 Bytes: checksum     --> the checksum of the packet.
  //   
  // Note that the payload is of fixed size equal to 37 bytes.
  int payload_size = 34;  
  uint8_t payload[payload_size];

  // File header must match the one on the sender side in order to know that the format is the same
  // file_header object is defined globally here.
  
  // Define the pos variable, which tracks the current position in the payload array.
  int pos = 0;
  
  // Load the header:
  for (int i=0;i<sizeof(file_header);i++) { payload[pos++] = file_header[i];}

  // Load the MAC of this device
  for (int i=0;i<sizeof(myMAC);i++) { payload[pos++] = myMAC[i];}

  // Load the the packet number as read in from the argument, "id"
  payload[pos++] = id >> 24;
  payload[pos++] = id >> 16;
  payload[pos++] = id >> 8;
  payload[pos++] = id;

  // Finally, load the checksum of the last packet read.
  payload[pos++] = checksum >> 24;
  payload[pos++] = checksum >> 16;
  payload[pos++] = checksum >> 8;
  payload[pos++] = checksum;
  
  // Verify that the pos ends up as the same size as the payload (it should be cause it's fixed)
  if (pos != payload_size) {
    //if (debug) Serial.println("handshake: Error, pos!=payload_size, probably fault in the header format");
    return 11;
  }

  // print out debug stuff
  //if (debug) {
  //  Serial.println("handshake: address to which packet is to be sent:");
  //  Serial.println("msb = " + String(msb,HEX));
  //  Serial.println("lsb = " + String(lsb,HEX));
    
  //  for (int i=0;i<sizeof(payload);i++) {
  //    Serial.println("payload["+ String(i) + "] = " + String(payload[i]));
  //  }
  //}

  // Send handshake, which if recived and verified should enable the next packet to be sent. 
  fid += 2;
  int err = send_packet(msb,lsb,payload,payload_size);
  if (err) {
    //Trap for errors...
    return err;
  }
  return 0;
} // End handshake()


/***********************************************************************************/
int read_file_from_xbee() {
  // In order to read a file, the first packet must conform to a specific format,
  //   which enable the contents of the file to be reconstructed from the set of 
  //   packets that follow the first packet.

  // Must have read packet. rx data struct should contain the first packet.
  int err = process_first_packet();
  if (err != 0) {
    //Trap for errors
    return 1;
  }

  // Open the file
  if (SD.exists(filename)) SD.remove(filename);
  myFile = SD.open(filename, FILE_WRITE);

  //if (debug) Serial.println("finished processing first packet.");
  // Should have processed first packet and have all the info needed to reconstruct
  //   the file from the remaining N packets.
  //Serial.println("There are "+String(num_packets) + " packets");
  int cycle = 0;
  int max_cycles = 5;
  int result;
  for (uint32_t ip=0;ip<num_packets;ip++) {

    //Serial.println("about to call process_packet for packet " + String (ip));
    // After handshake, the sender will transmit the next packet.
    // process_packet(ip), will read and process the ip_th packet.
    cycle = 0;
    result = process_packet(ip);
    while (result && (max_cycles < cycle)) {
      // Try again
      cycle++;
      result = process_packet(ip);
    }
    if (result) { // Error state
      return result;
    }
    myFile.flush();
  }
  myFile.close();
  Serial.println ("Successfully read file. closed myFile");
  return 0;
} // read_file_from_xbee()

/***********************************************************************************/
void setup() {
  Serial.begin(9600);  
  xbee.begin(Serial); 
  pinMode(LED_OUTPUT, OUTPUT); 
  pinMode(errorPin,OUTPUT);
  pinMode(4, OUTPUT);
  if (!SD.begin(4)) {
    //Serial.println("initialization failed!");
    return;
  }  
  
  // Get the MAC address of this device - need to include this in all packets.
  get_MAC();  // call to load myMAC with the MAC of this xbee.

}


/***********************************************************************************/

void loop() {
  // Call function to read file and store to SD. 
  loop_err = read_file_from_xbee();
  if (loop_err != 0) {
    //Trap for Errors
    if (debug) Serial.println("loop: err state returned from read_file_from_xbee() call, resetting");
  }
  delay(10);
}

