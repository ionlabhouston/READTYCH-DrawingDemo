#ifndef _BASIC_TYPES_H
#define _BASIC_TYPES_H

#include <stdlib.h>     //exit()
#include <string>

/**
 *
 * \struct DeviceInfo
 * 
 * \brief
 * 
 * This struct contains data about the various devices in the system such as displays,
 * tablets, etc. The information is supplied by command line arguments, configuration
 * files, etc.
 *
 */

struct DeviceInfo{
    int displayWidth;
    int displayHeight;
    double xScale;
    double yScale;
    double xOffset;
    double yOffset;
    int touchdownHeight;
    int buttonMode;
    std::string i2cBusName;
    int refreshRateMS;
    int refreshRateUS;
    bool transposeXY;
    bool flipX;
    bool flipY;
    int debugMode;
};

/**
 *
 * \struct TabletData
 * 
 * \brief
 * 
 * This struct contains data for the pen location and height as reported by the tablet.
 * The data is already transformed for use in positioning the virtual mouse.
 *
 */

struct TabletData{
    double transformedX;
    double transformedY;
    bool   touchDown;
};


enum ButtonModes {
    
    SINGLE_CLICK = 0,
    TRACK_ACTION = 1
    
};

enum DebugModes {
    
    DEBUG_OFF = 0,
    PRINT_TABLET_DATA = 1

};


#endif // _BASIC_TYPES_H
