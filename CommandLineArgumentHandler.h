#ifndef _COMMAND_LINE_ARGUMENT_HANDLER_H
#define _COMMAND_LINE_ARGUMENT_HANDLER_H

#include "../lib/Wacom/BasicTypes.h"

#include <vector>

//-------------------------------------------------------------------------------------------------
// class CommandLineArgumentHandler
//
/**
 * 
 *
 * 
 * 
*/

class CommandLineArgumentHandler
{

public:

    CommandLineArgumentHandler(DeviceInfo *pDeviceInfo);

    void init();

    int processArguments(int pArgc, char *pArgv[]);

protected:

    std::vector<std::string> splitString(std::string pInput, char pSeparator);

    void printBadCommandLineArgumentsError();

    int convertStringToDouble(std::string pString, double *pValue);

    int convertStringToInt(std::string pString, int *pValue);

    int handleDoubleArgValue(std::vector<std::string> pArgSet, std::string pArgTag, double *pValue);

    int handleIntArgValue(std::vector<std::string> pArgSet, std::string pArgTag, int *pValue);

    int handleBooleanArgValue(std::vector<std::string> pArgSet, std::string pArgTag, bool *pValue);

    int handleStringArgValue(std::vector<std::string> pArgSet, std::string pArgTag, std::string *pValue);

    int parseArgument(char *pArgument);

    int handleHelpOption(std::string pArg);

protected:

    DeviceInfo *deviceInfo;


};// end of class WacomI2CHandler
//-------------------------------------------------------------------------------------------------

#endif // _COMMAND_LINE_ARGUMENT_HANDLER_H