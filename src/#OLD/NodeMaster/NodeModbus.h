/*****************************************************************************
 *
 *  NodeModbus - MODBUS/RS-485 Implementation
 *
 *  This library establishes an RS-485 connection and calls
 *  functions that retrieve data from the Camera Node.
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
 * \brief NodeModbus Modbus class
 * \defgroup lt_setup NodeModbus Object Instantiation/Initialization
 * \defgroup lt_regcodes NodeModbus Register Codes
 * \defgroup lt_measure NodeModbus Measurement Functions
 */
#ifndef NodeModbus_h
#define NodeModbus_h

#if defined (PARTICLE)
  #include "application.h"
#elif defined (ARDUINO) && ARDUINO >= 100
	#include <Arduino.h>
	#include "CustomSoftwareSerial.h"
  #include <Time.h>
  #include <TimeLib.h>
#endif

#include "ModbusMaster.h"

/**
 * Modbus class for access to NodeModbus device.
 */

class NodeModbus {
  public:

    int TXpin;
    uint8_t loggingRate;
    uint8_t xmitdelay;
	uint8_t defaultResponseTimeout;
    int measurementdelay;
    bool _debug;

    union
    {
      uint16_t u[2];
      int i;
    } intData;
	
	union
    {
      uint16_t u[2];
      unsigned int ui;
    } uintData;
	
	union
    {
      uint16_t u[2];
      unsigned long ul;
    } ulongData;

    union
    {
      uint16_t u[2];
      float f;
    } floatData;

    union
    {
      uint16_t u[2];
      time_t t;
    } timeData;

    union
    {
      uint8_t u[64];
      char c[32];
    } stringData;

    // TODO: Create ISR for 6.14 Device Status - pg. 21
    // If there is an alarm, the device should wake up.
    // Look if there is a sensor pin that can act as a
    // hardware interrupt

    // Reference pg. 17- 19
	
	
	/** Initialize NodeModbus object
	 *
     * \param[in] RX/TX control pin
	 * \ingroup lt_setup
     */
    NodeModbus(int);
	
	/** Sets MODBUS transaction reponse timeout
	 *
	 * \param[in] Response timeout in ms
	 * \ingroup lt_setup
	 */
	void setMBResponseTimeout(uint8_t);
	
	/** Gets MODBUS transaction response timeout
	 *
	 * \return  Response timeout in ms
	 * \ingroup lt_setup
	 */
	uint8_t getMBResponseTimeout(void);
	
	/** Creates MODBUS serial connection to NodeModbus device.  
	 *  Checks if measurements can be made. 
	 *
	 * \param[in] Delay in retrieving data from MB response buffer in ms
	 * \param[in] Modbus response timeout in ms
	 * \return true on successful MODBUS connection
	 * \ingroup lt_setup
	 */
	bool LTbegin(uint8_t, uint8_t);
	
	/** Creates MODBUS serial connection to NodeModbus device.  
	 *  Checks if measurements can be made. 
	 *
	 * \return true on successful MODBUS connection
	 * \ingroup lt_setup
	 */
    bool LTbegin(void);

	/** Enable serial debug output for NodeModbus
	 * \ingroup lt_setup
	 */
    void enableDebug(void);
	
	/** Disable serial debug output for NodeModbus
	 * \ingroup lt_setup
	 */
    void disableDebug(void);

	/** Gets 13 character TGWC ID 
	 * \return String 13 character TGWC ID
	 * \ingroup lt_measure
	 */
    String   getTGWC(void);
    



  private:
  
    ModbusMaster _node;
	
	/** Gets NodeModbus register data via MODBUS transaction
	 *
	 * \param[in] Register ID
	 * \param[in] Number of registers
	 * \param[out] Pointer to Data RX buffer
	 * \return true on successful register data retrieval
	 * \ingroup lt_measure
	 */
	 
    bool _getRegisterData(uint16_t, uint16_t, uint16_t*);
	
	/** Sets NodeModbus register data via MODBUS transaction
	 *
	 * \param[in] Register ID
	 * \param[in] Number of registers
	 * \param[out] Pointer to Data RX buffer after setting 
	 * \return true on successfully setting register data
	 * \ingroup lt_measure
	 */
	 
    bool _writeRegisterData(uint16_t, uint16_t, uint16_t*);
    
    float _convertHEXtoFloat(uint16_t*);
    String _convertHEXtoString(uint16_t*);
    time_t _convertHEXtoTime(uint16_t*);
	unsigned int _convertHEXtoUINT(uint16_t*);

    void _convertStringToHEX(String, uint16_t*);
};


#endif
