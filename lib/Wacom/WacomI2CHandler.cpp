/******************************************************************************
* Title: WacomI2CHandler.cpp
* Author: Mike Schoonover
* Date: 12/05/23
*
*
*
*
*/

#include "WacomI2CHandler.h"

#include <stdio.h>
#include <iostream>
#include <fcntl.h>          // used for O_RDWR
#include <unistd.h>         // used for open(), close()
#include <sys/ioctl.h>      // used for ioctl()
#include <linux/i2c-dev.h>  // used for I2C_SLAVE

struct FeatureSupport {
	bool distance;
	bool tilt;
};

struct WacomFeatures {
	struct FeatureSupport support;
	int16_t x_max;
	int16_t y_max;
	int16_t pressure_max;
	int16_t distance_max;
	int16_t tilt_x_max;
	int16_t tilt_y_max;	
    int16_t fw_version;  //debug mks ~ this was a char in wacom_i2c.c but read as an integer from packet?
	unsigned char generation;
};

WacomFeatures features;

const int WACOM_BG9	= 0;        // G9 or earlier neither height nor tilt is supported
const int WACOM_AG12 = 1;	    // after G12 the IC supports "height" which is "ABS_DISTANCE" event
const int MAX_LEN_BG9 = 10;	    // packet length for G9 or earlier
const int MAX_LEN_G12 = 15; 	// packet length for G12
const int MAX_LEN_AG14 = 17;	// packet length for G14 or later

const int DISTANCE_MAX = 255;

const int WACOM_COMMAND_LSB	= 0x04;
const int WACOM_COMMAND_MSB	= 0x00;

const int WACOM_DATA_LSB = 0x05;
const int WACOM_DATA_MSB = 0x00;

const int REPORT_FEATURE = 0x30;

const int OPCODE_GET_REPORT	= 0x02;

const int WACOM_QUERY_REPORT = 3;
const int WACOM_QUERY_SIZE = 22;

const int WACOM_TIP_SWITCH          = 0x01;
const int WACOM_BARREL_SWITCH       = 0x02;
const int WACOM_ERASER              = 0x04;
const int WACOM_INVERT              = 0x08;
const int WACOM_BARREL_SWITCH_2     = 0x10;
const int WACOM_IN_PROXIMITY        = 0x20;

// from linuxwacom project file input-event-codes.h

#define BTN_TOOL_PEN		0x140
#define BTN_TOOL_RUBBER		0x141

//-------------------------------------------------------------------------------------------------
// class WacomI2CHandler::WacomI2CHandler (constructor)
//
/**
 * 
 * \class WacomI2CHandler
 * 
 * \brief
 * 
 * This class communicates with a Wacom tablet via an I2C bus.
 * 
 * For the Wacom WEZ01 Pen Digitizer Unit:
 * 
 *  x max = 15624
 * 
 *  y max = 20832
 * 
 * For a 1920 x 1080 display use command line arguments:
 *  
 *  --transposeXY=true
 *  --flip_x=false
 *  --flip_y=true
 *  --x_scale = 1920 / 20832 = 0.0921659
 *  --y_scale = 1080 / 15624 = 0.06912442 
 *
 * @param pDeviceInfo   a DeviceInfo instance containing info for devices in the system
 * @param pTabletData   a TabletData instance in which data retrieved from the tablet is
 *                      to be stored for access by other classes
 *
 */

WacomI2CHandler::WacomI2CHandler(DeviceInfo *pDeviceInfo, TabletData *pTabletData) : 
    deviceInfo(pDeviceInfo),
    tabletData(pTabletData),
    i2cFileHandle(-1),
    inRange(false)
{


}// end of WacomI2CHandler::WacomI2CHandler
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// class WacomI2CHandler::init
//
/**
 * 
 * Initializes the object. Should be called immediately after construction.
 * 
 * Does NOT start communications with the tablet.
 * 
 */

void WacomI2CHandler::init()
{

}// end of WacomI2CHandler::init
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// class WacomI2CHandler::start
//
/**
 * 
 * Opens the I2C link and prepares the table for use.
 * 
 * @param pI2CDevName   the name of the I2C bus to use: i2c-1, i2c-4, etc.
 * @param pI2CAddress   the address of the tablet on the I2C bus
 * 
 * @return              0 on success, -1 on error
 * 
 */

int WacomI2CHandler::start(const char * const pI2CDevName, int pI2CAddress)
{

    int status = openI2CConnection(pI2CDevName, pI2CAddress);

    return(status);

}// end of WacomI2CHandler::start
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// class WacomI2CHandler::stop
//
/**
 * 
 * Closes the I2C connection and stops all operations.
 * 
 * @return              0 on success, -1 on error
 * 
 */

int WacomI2CHandler::stop()
{

    int status = closeI2CConnection();

    return(status);

}// end of WacomI2CHandler::stop
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// class WacomI2CHandler::openI2CConnection
//
/**
 * 
 * Opens the I2C link for communication with the tablet.
 * 
 * @param pI2CDevName   the name of the I2C bus to use: i2c-1, i2c-4, etc.
 * @param pI2CAddress   the address of the tablet on the I2C bus
 * 
 * @return              0 on success, -1 on error
 * 
 */

int WacomI2CHandler::openI2CConnection(const char * const pI2CDevName, int pI2CAddress)
{

    // open I2C device

    i2cFileHandle = open(pI2CDevName, O_RDWR);

    if (i2cFileHandle < 0) {
        std::cerr << "Failed to open the I2C bus." << std::endl;
        return(-1);
    }

    // set the I2C device address for the connection...this will be used in all subsequent actions

    if (ioctl(i2cFileHandle, I2C_SLAVE, pI2CAddress) < 0) {
        std::cerr << "Failed to acquire bus access and/or talk to slave." << std::endl;
        close(i2cFileHandle);
        return(-1);
    }

	return(0);

}// end of WacomI2CHandler::openI2CConnection
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// class WacomI2CHandler::closeI2CConnection
//
/**
 * 
 * Closes the I2C link for communication with the tablet.
 * 
 * @return              0 on success, -1 on error
 * 
 */

int WacomI2CHandler::closeI2CConnection()
{

    close(i2cFileHandle);

    return(0);

}// end of WacomI2CHandler::closeI2CConnection
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// class WacomI2CHandler::queryDeviceInfo
//
/**
 * 
 * Queries the tablet for device info.
 * 
 * @return              0 on success, -1 on error
 * 
 */

int WacomI2CHandler::queryDeviceInfo()
{

	uint8_t getQueryDataCmd[] = {
            WACOM_COMMAND_LSB,
            WACOM_COMMAND_MSB,
            REPORT_FEATURE | WACOM_QUERY_REPORT,
            OPCODE_GET_REPORT,
            WACOM_DATA_LSB,
            WACOM_DATA_MSB,
        };

    if (write(i2cFileHandle, getQueryDataCmd, sizeof(getQueryDataCmd)) != sizeof(getQueryDataCmd)){
        printf("Error: failed to send packet to I2C tablet!"); return(-1);
    } else {
        printf("Success: packet sent to I2C tablet.");
    }

    uint8_t data[WACOM_QUERY_SIZE];

    if (read(i2cFileHandle, data, sizeof(data)) != sizeof(data)) {
        std::cerr << "Failed to read device info from the tablet." << std::endl;  return(-1);
    } else {
        //std::cout << "Data received: " << data[0] << ", " << data[1] << std::endl;
        std::cout << "Success: device info packet read from I2C tablet." << std::endl;
    }

    parseQueryReport(data);

    return(0);

}// end of WacomI2CHandler::queryDeviceInfo
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// class WacomI2CHandler::queryPenData
//
/**
 * 
 * Queries the tablet for data about the pen position and state. This function should be called
 * in a loop or by an Interrupt Handler.
 * 
 * @return              0 on success, -1 on error
 * 
 */

int WacomI2CHandler::queryPenData()
{

    penData.tipSwitch = 0;
    penData.eraser = 0;
    penData.barrelSwitch = 0;
    penData.barrelSwitch2 = 0;
    penData.penX = 0;
    penData.penY = 0;
    penData.penPressure = 0;
    penData.penTiltX = 0;
    penData.penTiltY = 0;
    penData.hoverHeight = 0;
    penData.tool = 0;

    uint8_t data[WACOM_QUERY_SIZE];

    if (read(i2cFileHandle, data, sizeof(data)) != sizeof(data)) {
        std::cerr << "Failed to read pen info from the tablet." << std::endl;  return(-1);
    } else{
        /// printArray(data, sizeof(data)); //debug mks
    }

    // empirical testing shows that valid pen data packet header -> 0x13 0x00 0x1a
    // This is not documented in Wacom's (terrible) 'Product Specifications for Wacom WEZ01 Pen
    // Digitizer Unit' document, so the header might change for unknown reasons

    int header0 = data[0];
    int header1 = data[1];
    int header2 = data[2];

    // do not return with an error, just ignore the packet

    if(header0 != 0x13 || header1 != 0x00 || header2 != 0x1a){ return(0); }

	penData.tipSwitch = data[3] & WACOM_TIP_SWITCH;
	penData.eraser = data[3] & WACOM_ERASER;
	penData.barrelSwitch = data[3] & WACOM_BARREL_SWITCH;
	penData.barrelSwitch2 = data[3] & WACOM_BARREL_SWITCH_2;

    parseUnsignedIntegerFromTwoByesLE(data, 4, &penData.penX);
    parseUnsignedIntegerFromTwoByesLE(data, 6, &penData.penY);
    parseUnsignedIntegerFromTwoByesLE(data, 8, &penData.penPressure); //pressure value is only 12 least significant bits of the 16

	if (!inRange){ // the previous value? why the previous value? from linuxwacom project
	    penData.tool = (data[3] & (WACOM_ERASER | WACOM_INVERT)) ? BTN_TOOL_RUBBER : BTN_TOOL_PEN;
    }

	inRange = data[3] & WACOM_IN_PROXIMITY;

	if (features.generation) {
        parseIntegerFromTwoByesLE(data, 11, &penData.penTiltX);
        parseIntegerFromTwoByesLE(data, 13, &penData.penTiltY);
		
		//input_report_abs(input, ABS_TILT_X, tilt_x); // linuxwacom project method of sending data to GUI
		//input_report_abs(input, ABS_TILT_Y, tilt_y); // linuxwacom project method of sending data to GUI

		// retrieve pen hover height

		if (data[0] == MAX_LEN_G12) {
			penData.hoverHeight = data[10];
		} else if (data[0] >= MAX_LEN_AG14) {
            parseIntegerFromTwoByesLE(data, 15, &penData.hoverHeight);
			penData.hoverHeight = -penData.hoverHeight; // the output is negative, make it positive
		}

		// input_report_abs(input, ABS_DISTANCE, distance); // linuxwacom project method of sending data to GUI

	}

    if(deviceInfo->debugMode == PRINT_TABLET_DATA){
        printf("%7d , %7d, %7d\n", penData.penX, penData.penY, penData.hoverHeight);
    }

    prepareDataForMouseEmulator();

    return(0);

}// end of WacomI2CHandler::queryPenData
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// class WacomI2CHandler::prepareDataForMouseEmulator
//
/**
 *
 * Scales and offsets the x,y location data, processes touchdown state of the pen, etc.
 * Also transposes the x and y values if specified via the command line arguments.
 * 
 * Stores the values in the TabletData variable.
 *
 */ 


void WacomI2CHandler::prepareDataForMouseEmulator()
{

    int rawX = penData.penX;
    int rawY = penData.penY;
    int x, y;

    if(deviceInfo->flipX){
        if(rawX > features.x_max){ rawX = features.x_max; }
        rawX = features.x_max - rawX;
    }

    if(deviceInfo->flipY){
        if(rawY > features.y_max){ rawY = features.y_max; }
        rawY = features.y_max - rawY;
    }

    if(deviceInfo->transposeXY){
        x = rawY; y = rawX;
      } else {
        x = rawX; y = rawY;
      }

    tabletData->transformedX = (x * deviceInfo->xScale) + deviceInfo->xOffset;
    tabletData->transformedY = (y * deviceInfo->yScale) + deviceInfo->yOffset;
    
    if (penData.hoverHeight <= deviceInfo->touchdownHeight){
        tabletData->touchDown = true;
    } else{
        tabletData->touchDown = false;
    }

}// end of WacomI2CHandler::prepareDataForMouseEmulator
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// class WacomI2CHandler::parseQueryReport
//
/**
 * 
 * Parses the values in the Query Report returned by the tablet.
 * 
 * @param pData             the data array containing the packet retrieved from the tablet
 * @param pNumElements
 * 
 */

int WacomI2CHandler::parseQueryReport(uint8_t *pData)
{

    parseIntegerFromTwoByesLE(pData, 3, &features.x_max);
    parseIntegerFromTwoByesLE(pData, 5, &features.y_max);
    parseIntegerFromTwoByesLE(pData, 11, &features.pressure_max);
    parseIntegerFromTwoByesLE(pData, 13, &features.fw_version);
	features.distance_max = pData[16];
    parseIntegerFromTwoByesLE(pData, 17, &features.tilt_x_max);
    parseIntegerFromTwoByesLE(pData, 19, &features.tilt_y_max);
    
    // debug mks - this was duplicated in the wacom_i2c.c code
    // what should it be reading?
    // features.distance_max = pData[16];
	
	if (features.distance_max){ features.support.distance = true; }

	if ((features.tilt_x_max && features.tilt_y_max)) { features.support.tilt = true; }

    // added by MKS to code from wacom_i2c.c because it might not get set from the
    // if-else-if statement below

    features.generation = WACOM_BG9;

    if (!features.support.distance && !features.support.tilt){
		features.generation = WACOM_BG9;
    }
	else if (features.distance_max == DISTANCE_MAX){
		features.generation = WACOM_AG12;
    }

    return(0);

}// end of WacomI2CHandler::parseQueryReport
//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// WacomI2CHandler::parseIntegerFromTwoByesLE
//
 /**
  * 
  * Extracts a two-byte signed integer from array pBuffer starting at index pIndex in the array. The
  * integer is reconstructed from the two data bytes at that index position.
  *
  * The value is parsed using Little Endian (LE) order (LSB first).
  *
  * @param pBuffer  the data array from which the value is to be parsed
  * @param pIndex	the starting position in pBuffer of the value to be parsed
  * @param pValue	the value extracted from the buffer will be returned via this pointer
  *
  * @return			the position of the byte following the two parsed in this call; can be used
  *                  to parse the next value in the array
  *
  */

int WacomI2CHandler::parseIntegerFromTwoByesLE(uint8_t *pBuffer, int pIndex, int16_t *pValue)
{

    int16_t byte1 = pBuffer[(pIndex)++];
    int16_t byte2 = pBuffer[(pIndex)++]; 

    *pValue = (byte1 & 0xff) + (byte2<<8 & 0xff00);

	return(pIndex);

}//end of WacomI2CHandler::parseIntegerFromTwoByesLE
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// WacomI2CHandler::parseUnsignedIntegerFromTwoByesLE
//
 /**
  * 
  * Extracts a two-byte unsigned integer from array pBuffer starting at index pIndex in the array. The
  * unsigned integer is reconstructed from the two data bytes at that index position.
  *
  * The value is parsed using Little Endian (LE) order (LSB first).
  *
  * @param pBuffer  the data array from which the value is to be parsed 
  * @param pIndex	the starting position in pBuffer of the value to be parsed
  * @param pValue	the value extracted from the buffer will be returned via this pointer
  * 
  * @return			the position of the byte following the two parsed in this call; can be used
  *                  to parse the next value in the array
  *
  */

int WacomI2CHandler::parseUnsignedIntegerFromTwoByesLE(uint8_t *pBuffer, int pIndex, uint16_t *pValue)
{

    uint16_t byte1 = pBuffer[(pIndex)++];
    uint16_t byte2 = pBuffer[(pIndex)++]; 

    *pValue = (byte1 & 0xff) + (byte2<<8 & 0xff00);

	return(pIndex);

}//end of WacomI2CHandler::parseUnsignedIntegerFromTwoByesLE
//--------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// class WacomI2CHandler::printArray
//
/**
 * 
 * Prints the values of an array to the terminal.
 * 
 * @param pData             the array to be printed
 * @param pNumElements      the number of elements in the array to be printed
 * 
 */

int WacomI2CHandler::printArray(uint8_t *pData, int pNumElements)
{


    int i = 0;
    int numPerLine = 16;
    int countPerLine = 1;

    while(i < pNumElements){

        printf("%02x ", pData[i] );

        if(countPerLine == numPerLine){
            printf("\n"); 
            countPerLine = 1;
        } else{
            countPerLine++;
        }

        i++;

    }

    printf("\n\n");

    return(0);

}// end of WacomI2CHandler::printArray
//-------------------------------------------------------------------------------------------------
