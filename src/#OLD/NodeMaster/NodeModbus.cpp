/*****************************************************************************
 *
 *  NodeModbus - MODBUS/RS-485 Implementation
 *
 *  This library establishes an RS-485 connection and calls
 *  functions that retrieve data from the NodeModbus 500.
 *  
 *  Author: RSAPD Technical Consulting Ltd. (robbie@rsconsulting.ca)
 *  Created for: Smartifi
 *  
 *  Copyright 2017 RSAPD Technical Consulting Ltd.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *****************************************************************************/
 
 /**
  * \file
  * \brief Modbus class for access to NodeModbus device.
  */

#if defined (PARTICLE)
  #include "application.h"
#elif defined (ARDUINO) && ARDUINO >= 100
	#include <Arduino.h>
	#include "CustomSoftwareSerial.h"
  #include <Time.h>
  #include <TimeLib.h>

#endif
  
#include "NodeModbus.h"
#include  "ModbusMaster.h"

// PROBE COMMON REGISTERS
/** \ingroup lt_regcodes
  * @{
  */
  
#define REG_DEVICE_ID          		9001 // size 1 ushort
#define REG_DEVICE_STATUS      		9100 // size 1 bool  //read a coil function code 0x01

// NodeModbus Snapshot Registers	
#define REG_IMAGE_STORED_SEND		6001
#define REG_IMAGE_SNAP_SEND			6004

// NodeModbus Sensor calibration


// NodeModbus power registers - pg. 34

// COMMUNICATION REGISTERS          
#define REGISTER_SERIAL_COMM_CONFIG 9201 // size 1 u short

  /** @}*/

NodeModbus::NodeModbus(int txpin) :
							_node(1),
                            xmitdelay(300),
                            measurementdelay(0),
							
                            _debug(false){
  NodeModbus::TXpin = txpin;
  NodeModbus::defaultResponseTimeout = getMBResponseTimeout();
  
}



bool NodeModbus::begin(){
	return NodeModbus::begin(NodeModbus::xmitdelay, _node.ku8MBResponseTimeout);
}

bool NodeModbus::begin(uint8_t xmitdelay, uint8_t modbusTimeout){

  int i=0;
  bool result = false;
  int startTime, stopTime;
  bool getDeviceStatus;
  
  if(xmitdelay > 0 && modbusTimeout > 0){
	  NodeModbus::xmitdelay = xmitdelay;
	  NodeModbus::setMBResponseTimeout(modbusTimeout);
  }

  _node.enableTXpin(TXpin);
  
  // Preset to 19200 in ModbusMaster
  NodeModbus::_node.begin(19200);
  



	startTime = millis();
     TGWC_ID = getDeviceStatus();
     delay(measurementdelay);

     
  	 if(levelVal <= -200 || tempVal<=-200){
		result = false;
  	  }
      else{
          
        if(NodeModbus::_debug){
          
          Serial.print(F("LT: Transmit delay calibrated value = "));
          Serial.println(xmitdelay);
          Serial.print(F("LT: Measurement delay calibrated value = "));
          Serial.println(measurementdelay);
		  Serial.print(F("LT: ModbusMaster Response Timeout = "));
          Serial.println(NodeModbus::getMBResponseTimeout());
        
          Serial.print(F("LT: ITERATION: "));
          Serial.print(i);
          Serial.print(F(" Temperature: "));
          Serial.print(tempVal);
          Serial.print(F(" Level: "));
          Serial.print(levelVal);
          Serial.print(F(" Level reference: "));
          Serial.print(levelRef);
          Serial.print(F(" TGWC ID: "));
          Serial.print(TGWC_ID);
          Serial.print(F(" Current Time: "));
          Serial.print(timeVal);
		  Serial.printlnf(" Acquisition Time: %d", stopTime-startTime);
          
        }
		result = true;
        i++;
      }
    
	if(result){
		Serial.println("LT: SUCCESS -> NodeModbus initialization ");
	}
	else{
		Serial.println("LT: FAIL -> NodeModbus initialization ");
	}
	
	return result;
  
}

void NodeModbus::enableDebug(void){
  NodeModbus::_debug = true;
  NodeModbus::_node.enableDebug();
  Serial.println(F("LT: Debug enabled"));
}

void NodeModbus::disableDebug(void){
  NodeModbus::_debug = false;
  NodeModbus::_node.disableDebug();
  Serial.println(F("LT: Debug disabled"));
}

void NodeModbus::setMBResponseTimeout(uint8_t t){
	_node.ku8MBResponseTimeout = t;
}

uint8_t NodeModbus::getMBResponseTimeout(){
	return _node.ku8MBResponseTimeout;
}


float NodeModbus::getPressure(void){

  uint16_t data[2];

  if(_getRegisterData(REG_MEASURE_PRESSURE, 2, data)){

    if(_debug){
      Serial.println(F("Pressure read successfull"));
    }
    return _convertHEXtoFloat(data);
  }
  else{
    Serial.println(F("Pressure read error"));

  }
  return -200;


}

float NodeModbus::getTemperature(void){

  uint16_t data[2];

  if(_getRegisterData(REG_MEASURE_TEMP, 2, data)){

    if(NodeModbus::_debug){
      Serial.println(F("Temperature read successfull"));
    }
    return _convertHEXtoFloat(data);
  }
  else{
    Serial.println(F("Temperature read error"));
  }

  return -200;

}

unsigned int NodeModbus::getInternalVoltage(void){
  uint16_t data[1];

  if(_getRegisterData(REG_INTERNAL_VOLTAGE, 1, data)){

    if(NodeModbus::_debug){
      Serial.println(F("Internal Voltage read successfull"));
    }
	
	if(NodeModbus::_debug) Serial.printlnf("Internal Voltage is:  %d mV", data[0]);
    return data[0];
  }
  else{
    Serial.println(F("Internal Voltage read error"));
  }

  return 0;
}

unsigned int NodeModbus::getExternalVoltage(void){
	
  uint16_t data[1];

  if(_getRegisterData(REG_EXTERNAL_VOLTAGE, 1, data)){

    if(NodeModbus::_debug){
      Serial.println(F("External Voltage read successfull"));
    }
	
	if(NodeModbus::_debug) Serial.printlnf("External Voltage is:  %d mV", data[0]);
    return data[0];
  }
  else{
    Serial.println(F("External Voltage read error"));
  }

  return 0;
}

float NodeModbus::getInternalBatteryPercentRemaining(void){
	
	uint16_t data[2];

	unsigned int usedBatteryTicks, totalBatteryTicks;

	if(_getRegisterData(REG_USED_BATTERY_TICKS, 2, data)){

		if(NodeModbus::_debug){
		  Serial.println(F("Used battery Ticks read successfull"));
		}
		
		usedBatteryTicks = _convertHEXtoUINT(data);		
	}
	else{
		Serial.println(F("Used battery Tick read error"));
		return 0;
	}


	if(_getRegisterData(REG_TOTAL_BATTERY_TICKS, 2, data)){
		
		if(NodeModbus::_debug){
		  Serial.println(F("Total battery Ticks read successfull"));
		}
		
		totalBatteryTicks = _convertHEXtoUINT(data);
	
	}
	
	else{
		Serial.println(F("Total battery Tick read error"));
		return 0;
	}
	
	
	unsigned int diff = totalBatteryTicks - usedBatteryTicks;
	
	if(NodeModbus::_debug){ 
		Serial.printlnf("Total battery ticks: %u, Used battery ticks: %u, Difference: %u", totalBatteryTicks, usedBatteryTicks, diff);
	}
	
	
	float battery_pct =  (1.0f-((float)diff/(float)totalBatteryTicks))*100.0f;
	
	if(NodeModbus::_debug) Serial.printlnf("Battery remaining: %.5f %%", battery_pct);
	
	return battery_pct;
	
}

unsigned int NodeModbus::getDeviceSerial(void){
	
	uint16_t data[2];
	
	//HEX: 00 02 F3 66
	//DeviceSerial: 193382
	if(_getRegisterData(REG_DEVICE_SERIAL, 2, data)){

		if(NodeModbus::_debug){
		  Serial.println(F("Device Serial read successfull"));
		}

		return _convertHEXtoUINT(data);		
	}
	else{
		Serial.println(F("Device Serial read error"));
		return 0;
	}
	
}



#if !defined (PARTICLE)
void NodeModbus::getMeasurement(MeasurementType mType, MeasurementData mData){

  

  // Check to see if measurement var passed is valid

  switch(mType){
    case PRESSURE:
    case TEMPERATURE:
    case WATERLEVEL:
    break;
    default:
    Serial.println(F("LT: Unknown measurement type"));
    return;
  }

  uint16_t* dataBuffer;
  
  for(unsigned i=0; i<ArraySize(_rgMeasuredParameterTable); ++i){
 
      rgRegister rgRegItem;
      
      PROGMEM_readAnything(&_rgMeasuredParameterTable[i],rgRegItem);
      
      Serial.print(F("ITERATION: "));
      Serial.println(i);

      Serial.print(F("Dynamic allocated data size: "));
      Serial.println(rgRegItem.dataType);
  
      dataBuffer = new uint16_t[rgRegItem.dataType];
      //uint16_t dataBuffer[rgRegItem.dataType];
  
      if(dataBuffer = nullptr){
        Serial.println("LT: Error: Could not allocate memory");
        continue;
      }

      Serial.print(F("Register Address: "));
      Serial.println(((uint16_t)(8*mType)+rgRegItem.regAddress));
      Serial.print("Register Size: ");
      Serial.println(rgRegItem.regSize);

      
      delay(measurementdelay);
      _getRegisterData((uint16_t)(8*mType)+rgRegItem.regAddress, rgRegItem.regSize, dataBuffer);
      delay(measurementdelay);


      Serial.print(rgRegItem.desc);
      Serial.print(": ");
      
      switch(i){
        case 0:
          //disableDebug();
          mData.measuredValue = _convertHEXtoFloat(dataBuffer);
          Serial.println(mData.measuredValue);
          break;
        case 1:
          //enableDebug();
          _processID(_convertHEXtoUSHORT(dataBuffer), &mData.paramID, PARAMETER);
          Serial.print("-conv:");
          //Serial.println(mData.paramID.id);
          Serial.println(_convertHEXtoUSHORT(dataBuffer));
          break;
        /*  
        case 2:
          //enableDebug();
          _processID(_convertHEXtoUSHORT(dataBuffer), &mData.unitID, UNIT);
          Serial.println(mData.unitID.id);
          break;
        case 3:
          //enableDebug();
          _processID(_convertHEXtoUSHORT(dataBuffer), &mData.dqID, DATAQUALITY);
          Serial.println(mData.dqID.id);
          */
        case 4:
          //disableDebug();
          mData.offlineSentinelValue =  _convertHEXtoFloat(dataBuffer);
          Serial.println(mData.offlineSentinelValue);
          break;
        case 5:
          // Write avail units process function
          //disableDebug();
          mData.availableUnits = dataBuffer;
          Serial.println(mData.availableUnits, HEX);
          break;
      }
      
      delete[] dataBuffer;
  }
} 

bool NodeModbus::_processID(uint16_t* data, rgDescriptorID* _location,  rgIDTYPE _rgIDTYPE){
  unsigned short id = _convertHEXtoUSHORT(data);
  bool result = false;
  rgDescriptorID* rgIDTable;

  switch(_rgIDTYPE){
        
        case UNIT:
          //rgIDTable = new rgDescriptorID[ArraySize(_rgUnitID)];
          rgIDTable = pgm_read_ptr(&_rgUnitID);
          //rgIDTable = PROGMEM_getAnything(&_rgUnitID);
          break;
        case DATAQUALITY:
          rgIDTable = pgm_read_ptr(&_rgUnitID);
          //rgIDTable = new rgDescriptorID[ArraySize(&_rgDQID)];
          //rgIDTable = PROGMEM_getAnything(&_rgDQID);
          break;
        case PARAMETER:
          rgIDTable = pgm_read_ptr(&_rgUnitID);
          //rgIDTable = new rgDescriptorID[ArraySize(&_rgParamID)];
          //rgIDTable = PROGMEM_getAnything(&_rgParamID);
          break;
        case EXCEPTION:
          rgIDTable = pgm_read_ptr(&_rgUnitID);
          //rgIDTable = new rgDescriptorID[ArraySize(&_rgExceptionID)];
          //rgIDTable = PROGMEM_getAnything(&_rgExceptionID);
          break;
          
 }
 
  for(unsigned i=0; i<(sizeof(rgIDTable)/sizeof(rgIDTable[0])); ++i){
    //rgDescriptorID rgID = 
    //PROGMEM_readAnything(rgIDTable[i], rgID);
    if(id == pgm_read_word(&rgIDTable[i].id)){
      memcpy_P(&_location, &rgIDTable[i], sizeof(rgDescriptorID));
      //PROGMEM_readAnything(rgIDTable[i], _location);
      result = true;
    }
  }
  
  //delete[] rgIDTable;
  
  if(!result)
    Serial.println(F("LT: _processID NOT found)"));
  
  return result;
}

#endif

float NodeModbus::getLevel(void){

  uint16_t data[2];
  
  if(_getRegisterData(REG_MEASURE_LEVEL, 2, data)){

    if(NodeModbus::_debug){
      Serial.println(F("Level read successfull"));
    }
    return _convertHEXtoFloat(data);
  }
  else{
    Serial.println(F("Level read error"));
  }

  return -200.0;

}

float NodeModbus::getLevelReference(void){

  uint16_t data[2];

  if(_getRegisterData(REG_CALIBRATE_LR, 2, data)){

    if(NodeModbus::_debug){
      Serial.println(F("Level reference read successfull"));
    }
    return _convertHEXtoFloat(data);
  }
  else{
    Serial.println(F("Level reference read error"));
  }

  return -200.0;
  
}

bool NodeModbus::setLevelReference(float lref){

  floatData.f = lref;
  uint16_t d[2] = {floatData.u[1], floatData.u[0]};

  // For debugging purposes

  if(NodeModbus::_debug){
	  Serial.print(F(" floatData[0]: "));
	  Serial.print(d[0],HEX);
	  Serial.print(F(" floatData[1]: "));
	  Serial.print(d[1],HEX);
	  Serial.print(F(" floatData.f: "));
	  Serial.println(floatData.f);
  }

   _writeRegisterData(REG_CALIBRATE_LR, 2, d);

   delay(measurementdelay);

   if(getLevelReference() == lref){

        if(NodeModbus::_debug){
              Serial.print(F("LT: Level reference sucessfully set to: "));
              Serial.println(lref);
        }
        return true;
      }
  

  Serial.printlnf("LT: ERROR occurred while setting Level reference to: %.2f m",lref);
  return false;
}

/** Gets the NodeModbus TGWC ID from 32 registers of the REG_DEVICE_NAME 
 *
 * \return String TGWC_ID
 */
 
String NodeModbus::getTGWC(void){
    uint16_t data[32];
	
    
    if(_getRegisterData(REG_DEVICE_NAME, 32, data)){

      // Convert from uint16_t to string characters
	  
	  String result = _convertHEXtoString(data);
	  
	  if(NodeModbus::_debug) Serial.printlnf("LT: TGWC ID read successfully. ID: %s",result.c_str());
	  
	  return result;
    }

    return String("MXXXXX.XXXXXX");
}

bool NodeModbus::setTGWC(String TGWC){

  char c_buf[14] = {0};
  uint16_t u_buf[32] = {0};
  
  // Validating TGWC id
  if(!validateTGWC(TGWC.c_str())){
	  if(NodeModbus::_debug) Serial.println("LT: TGWC provided is not valid ");
	return false;
  }
  
  TGWC.toCharArray(c_buf,14);

  for(int i=0; i<13; i++){
   
    u_buf[i] = toupper(c_buf[i]);
    if(NodeModbus::_debug) Serial.print(c_buf[i]);
    if(NodeModbus::_debug) Serial.print(":");
    if(NodeModbus::_debug) Serial.print(u_buf[i], HEX);
    if(NodeModbus::_debug) Serial.print(" ");
	
  }
  if(NodeModbus::_debug) Serial.println(" ");
  

  if(_writeRegisterData(REG_DEVICE_NAME, 32, u_buf)){
      if(NodeModbus::_debug) Serial.println("LT: Device name write successful");
      return true;
  }
  
  if(NodeModbus::_debug) Serial.println("LT: FAIL: Device name write successful");
  return false;
  
}

bool NodeModbus::validateTGWC(const char* TGWC){
  
  char c_buf[14] = {'\0'};
  
  // Validate TGWC length is 13
  if(strlen(TGWC) != 13){ 
  
    if(NodeModbus::_debug) Serial.printlnf("LT: Length Received: %d. TGWC must be 13 characters long.",strlen(TGWC));
    return false;
  }  
  
  // Convert to uppercase alpha characters
  for(int i=0; i<13; i++){

	  c_buf[i] = toupper(TGWC[i]);
	  if(NodeModbus::_debug) Serial.printlnf("c_buf[%d]=%c; TGWC[%d]=%c",i,c_buf[i],i,TGWC[i]);
    
  }
  
  if(NodeModbus::_debug) Serial.printlnf("c_buf=%s; TGWC=%s",c_buf,TGWC);
  
  // Validate 1st character is M
  if(c_buf[0] !='M'){
    if(NodeModbus::_debug) Serial.println(F("TGWC must start with M")); 
    return false;
  }
  
  // Validate 2nd character is (B|D|[0-9])
  
  if(!isdigit(c_buf[1])){
	  if(c_buf[1] != 'B'){
		  if(c_buf[1] != 'D'){
			  if(NodeModbus::_debug) Serial.printlnf("TGWC must start with M[0-9], MB, or MD. char at 1 is: %c, ", c_buf[1]); 
			  return false;	  
		 }
	  }
  }
 
  /*** Alternate version of the above 
  
  // Validate 2nd character is (B|D|[0-9])
  if(!isdigit(c_buf[1]) && !(c_buf[1] == 'B' || c_buf[1] == 'D')){

	  Serial.printlnf("TGWC must start with M[0-9], MB, or MD. char at 1 is: %c, ", c_buf[1]); 
	  return false;	  

  }
  ***/
  
  // Validate 7th character is a period (.)
  if(c_buf[6] != '.'){
    if(NodeModbus::_debug) Serial.println(F("TGWC must be separated by a period at position 7.")); 
    return false;
  }
  
  // Validate remaining characters are digits
  
  for(int i=0; i<13; i++){
    
    if(i > 1 && i != 6 && !isDigit(c_buf[i])){
      if(NodeModbus::_debug) Serial.printlnf("TGWC does not have a digit in position: %d",i+1);
      return false;
    }
  }
  
  
  return true;
  
}

String NodeModbus::convertToTimeStamp(time_t t){
  //char ts[100];
  //sprintf(ts, "%d-%d-%d 
  return Time.timeStr(t);
}

time_t NodeModbus::getDeviceTime(void){
    uint16_t data[3];
    
    if(_getRegisterData(REG_CURRENT_TIME, 3, data))
    {

      timeData.u[0] = data[1];
      timeData.u[1] = data[0];

      if(NodeModbus::_debug){
        Serial.println(" ");
        Serial.print("timeData.t: ");
        Serial.println(timeData.t);
      }

      if(NodeModbus::_debug){
        Serial.println(F("Device time read successfull"));
      }
      
      return timeData.t;

    }
    else{
      Serial.println(F("Could not get the time"));
    }

}

// TODO: Create implementation of NodeModbus::setDeviceTime
bool NodeModbus::setDeviceTime(time_t devTime){

  //timeData.t = devTime;
  return true;
}

// TODO: Create implementation of NodeModbus::getDeviceStatus
uint8_t NodeModbus::getDeviceStatus(void){
	
	uint16_t data[2];
	
	if(_getRegisterData(REG_DEVICE_STATUS,2,data)){
		
		// process data
		
		/**
		* alarm category bits:  0,3,5,10,11 (100105000011) = 0x9430 + 0x0000
		* status category bits: 8,9,12 (0000 0000 1100 1000 0000 0000 0000 0000) = 0xC800 + 0x0000
		* warning category bits: 1,2,4,13,14 (0110 1000 1100 0110 0000 0000 0000 0000) = 0x68C6 + 0x0000
		*/
		
		
	}
  
}
bool NodeModbus::_getRegisterData(uint16_t registerAddress, uint16_t regSize, uint16_t* data){

    uint8_t j, result;
	uint8_t defaultResponseTimeout = getMBResponseTimeout();

    if(NodeModbus::_debug){
      Serial.print(F("Reading register: "));
      Serial.print(registerAddress);
      Serial.print(F(" regSize: "));
      Serial.print(regSize);
      Serial.print(F(" sizeof(data): "));
      Serial.print(sizeof(&data));
      Serial.print(F(" xmitdelay: "));
      Serial.println(xmitdelay);
    }
	
	
	// Adjust response timeout for larger register sizes
	
	if(regSize >8){
		
		NodeModbus::setMBResponseTimeout(300);
	}
	// Delay and get register data.
    delay(xmitdelay);
    result = _node.readHoldingRegisters(registerAddress-1, regSize);
    delay(xmitdelay);
	
	if(regSize >8){
		NodeModbus::setMBResponseTimeout(NodeModbus::defaultResponseTimeout);
	}
	
	
	// LT is sleeping, ping it a couple more times.  
	
	if(result ==_node.ku8MBResponseTimedOut){

            if(NodeModbus::_debug){
              Serial.println(F("LT: Response timed out. Trying again. "));
            }	
			
			int i =0;
			
			while(i < 2){

				delay(xmitdelay);
				result = _node.readHoldingRegisters(registerAddress-1, regSize);
				delay(xmitdelay);
				
				if(NodeModbus::_debug){
					Serial.printlnf("LT: Timeout iteration# %d. ", i);
				}	
				
				if(result == _node.ku8MBResponseTimedOut){
					Serial.println(F("LT: Failed. Response timed out. Adjust the delay timings? "));
				}
				else break;
				
				i++;
				
			}

    }

    if (result == _node.ku8MBSuccess) {
      if(NodeModbus::_debug){
        Serial.print(F("LT: Success, Received data: "));
	  }

      for (j = 0; j < regSize; j++) {

        data[j] = _node.getResponseBuffer(j);
        if(NodeModbus::_debug){
          Serial.print(data[j], HEX);
          Serial.print(F(" "));
        }
      }
	  
	  if(NodeModbus::_debug){
		  Serial.println("");
	  }

	  _node.clearResponseBuffer();
	  _node.clearTransmitBuffer();

      return true;

    }
    else{

      Serial.print(F("LT: Failed, Response Code: "));
      Serial.println(result, HEX);
    }
	
	_node.clearResponseBuffer();
	_node.clearTransmitBuffer();

  return false;
}

bool NodeModbus::_writeRegisterData(uint16_t registerAddress, uint16_t regSize, uint16_t* data){
  
  uint8_t result;
  int i,j;
    
  for(j=0; j<regSize; j++){
	  
    if(NodeModbus::_debug){

      Serial.print(F(" j: "));
      Serial.print(j);
      Serial.print(F(" lwData: "));
      Serial.print(lowWord(data[j]), HEX);
      Serial.print(F(" hwData: "));
      Serial.println(highWord(data[j]), HEX);
    }
    
    if(_node.setTransmitBuffer(j, lowWord(data[j])) == _node.ku8MBIllegalDataAddress){
      Serial.println(F("LT: WriteRegisterData LowWord - Illegal address."));
      return false;
    }

  }
  
  if(NodeModbus::_debug){
    Serial.println(F("LT: Successfully set data in Transmit buffer"));
  }


  NodeModbus::setMBResponseTimeout(300);
	
	
  result = _node.writeMultipleRegisters(registerAddress-1, regSize); 
  delay(xmitdelay);

  NodeModbus::setMBResponseTimeout(NodeModbus::defaultResponseTimeout);
	

  if(result ==_node.ku8MBResponseTimedOut){

		if(NodeModbus::_debug){
		  Serial.println(F("LT: Response timed out. Trying again. "));
		}	
		
		int i =0;
		
		while(i < 2){

			delay(xmitdelay);
			result = _node.writeMultipleRegisters(registerAddress-1, regSize);
			delay(xmitdelay);
			
			if(NodeModbus::_debug){
				Serial.printlnf("LT: Timeout iteration# %d. ", i);
			}	
			
			if(result == _node.ku8MBResponseTimedOut){
				Serial.println(F("LT: Failed. Response timed out. Adjust the delay timings? "));
			}
			else break;
			
			i++;
			
		}

 }

	
  if (result == _node.ku8MBSuccess) {
      if(NodeModbus::_debug){
        Serial.print(F("LT: Successfully wrote register data. "));
    }
	
	for (j = 0; j < regSize; j++) {

        data[j] = _node.getResponseBuffer(j);
        if(NodeModbus::_debug){
          Serial.print(data[j], HEX);
          Serial.print(F(" "));
        }
      }
	  
	    
    _node.clearResponseBuffer();
    _node.clearTransmitBuffer();
	
	return true;

  }
  else{
	  
      Serial.print(F("LT: Failed writing, Response Code: "));
      Serial.println(result, HEX);
  }
  
      _node.clearResponseBuffer();
    _node.clearTransmitBuffer();
    

  return false;

  

}

float NodeModbus::_convertHEXtoFloat(uint16_t* data){

  for(int i=0; i<2; i++)
    floatData.u[1-i] = data[i];

  return floatData.f;

}

unsigned int NodeModbus::_convertHEXtoUINT(uint16_t* data){

		uintData.u[0] = data[1];
		uintData.u[1] = data[0];
		
		return uintData.ui;
}

String NodeModbus::_convertHEXtoString(uint16_t* data){

  String s;  

  for(int i=0; i<32; i++)
  {
    s.concat((char)data[i]);
  }
  s.trim();
  return s;
  
}

void NodeModbus::_convertStringToHEX(String s, uint16_t* d){  
  char c[s.length()];
  s.toCharArray(c,s.length());

  
  
 Serial.println(" ");
  
  for(int i=0; i<32; i++){

  }
  Serial.println(" ");
}

time_t NodeModbus::_convertHEXtoTime(uint16_t* data){
  // 3 register number
  // first 4 bytes represent time in seconds since 00:00:00 January 1, 1970 UTC
  
  for(int i=0; i < 3; i++)
      timeData.u[2-i] = data[i];

  return timeData.t;
}
