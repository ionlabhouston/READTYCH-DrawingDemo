#ifndef _WACOM_I2C_HANDLER_H
#define _WACOM_I2C_HANDLER_H


#include "BasicTypes.h"

#include <stdint.h>

struct PenData{

    uint16_t tipSwitch;
    uint16_t eraser;
    uint16_t barrelSwitch;
    uint16_t barrelSwitch2;
    uint16_t penX;
    uint16_t penY;
    uint16_t penPressure;
    int16_t penTiltX;
    int16_t penTiltY;
    int16_t hoverHeight;
    int tool;

};

//-------------------------------------------------------------------------------------------------
// class WacomI2CHandler
//
/**
 * 
 * This class handles communications with a Wacom tablet via an I2C interface.
 * 
 * 
*/

class WacomI2CHandler
{

public:

    WacomI2CHandler(DeviceInfo *pDeviceInfo, TabletData *pTabletData);

    void init();

    int start(const char * const pI2CDevName,  int pI2CAddress);

    int stop();

    int openI2CConnection(const char * const pI2CDevName, int pI2CAddress);

    int closeI2CConnection();

    int queryDeviceInfo();

    int queryPenData();

    int parseQueryReport(uint8_t *pData);

    int printArray(uint8_t *pData, int pNumElements);

    int parseIntegerFromTwoByesLE(uint8_t *pBuffer, int pIndex, int16_t *pValue);

    int parseUnsignedIntegerFromTwoByesLE(uint8_t *pBuffer, int pIndex, uint16_t *pValue);

    PenData *getPenDataPtr( ){ return(&penData); }

protected:

    void prepareDataForMouseEmulator();

protected:

    DeviceInfo *deviceInfo;

    TabletData *tabletData;

    int i2cFileHandle;

    bool inRange;   // the linuxwacom project refers to this as 'prox' but the wacom
                    // documenation uses the more descriptive 'In Range'

    PenData penData;

};// end of class WacomI2CHandler
//-------------------------------------------------------------------------------------------------

#endif // _WACOM_I2C_HANDLER_H