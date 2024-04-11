/******************************************************************************
* Title: CommandLineArgumentHandler.cpp
* Author: Mike Schoonover
* Date: 12/05/23
*
*
*
*
*/

#include "CommandLineArgumentHandler.h"

#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <string>
#include <float.h>

//-------------------------------------------------------------------------------------------------
// class CommandLineArgumentHandler::CommandLineArgumentHandler (constructor)
//
/**
 * 
 * \class CommandLineArgumentHandler
 * 
 * \brief
 * 
 * This class handles parsing of command line arguments passed in by the operating system. It
 * also handles displaying of help info if -?, -h, or --help are present as the first argument.
 * The values from the arguments are stored in the associated variables.
 *
 */

CommandLineArgumentHandler::CommandLineArgumentHandler(DeviceInfo *pDeviceInfo) :
    deviceInfo(pDeviceInfo)
{


}// end of CommandLineArgumentHandler::CommandLineArgumentHandler
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// class CommandLineArgumentHandler::init
//
/**
 * 
 * Initializes the object. Should be called immediately after construction.
 * 
 */

void CommandLineArgumentHandler::init()
{

}// end of CommandLineArgumentHandler::init
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// class CommandLineArgumentHandler::processArguments
//
/**
 * 
 * Processes the arguments in argv. The number of arguments is specified in argc.
 * 
 * Scans through all the command line arguments in the pArgv array for valid arguments and sets
 * the appropriate variable for each argument to the associated value.
 * 
 * Displays help info if option ?, -h, or --help is found as the first argument.
 * 
 *  
 * @param pArgc     the number of arguments in pArgv, plus 1 for the command line itself which
 *                  is stored in the array at index 0
 * 
 * @param pArgv     an array of CStrings containing the command line arguments to be parsed
 * 
 * 
 */

int CommandLineArgumentHandler::processArguments(int pArgc, char *pArgv[])
{

    if (pArgc < 2) {
        printBadCommandLineArgumentsError();
        return(-1);
    }

    int status;

    std::string argStr(pArgv[1]);

    status = handleHelpOption(argStr);
    if(status == -1){ return(status); }

    // skip index 0 as it contains the command line string and not an argument

    for(int i=1; i<pArgc; i++){

        status = parseArgument(pArgv[i]);
        if(status != 0){ return(status); }

    }

    return(0);

}// end of CommandLineArgumentHandler::processArguments
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// CommandLineArgumentHandler::splitString
//
/**
 * 
 * Splits string pInput into separate parts using pSeparator as the separation character.
 *
 * @param pInput        the string to be split
 * @param pSeparator    the character which separates the pieces to be split
 * 
 */

std::vector<std::string> CommandLineArgumentHandler::splitString(std::string pInput, char pSeparator)
{

    size_t pos;
    std::vector<std::string> splits;

    while (!pInput.empty()) {

        pos = pInput.find(pSeparator);

        if (pos == std::string::npos) {
            splits.push_back(pInput);
            break;
        }

        splits.push_back(pInput.substr(0, pos));

        pInput = pInput.substr(pos + 1);
    }
    
    return(splits);

}// end of CommandLineArgumentHandler::splitString
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// CommandLineArgumentHandler::printBadCommandLineArgumentsError
//
/**
 * 
 * Prints an error message regarding missing/invalid command line arguments.
 * 
 */

void CommandLineArgumentHandler::printBadCommandLineArgumentsError()
{

    std::string exampleCommand = 
    "wacom_i2c_daemon --x_scale=0.0921659 --y_scale=0.06912442 --x_offset=0 --y_offset=0";
    
    exampleCommand.append(" --transposeXY=true");

    exampleCommand.append(" --touchdown_height=5 --i2c_bus=i2c-1 --refresh_rate_MS=100");

    std::cerr << "Some options are missing or invalid. Example for a 1920x1080 display:\n";
    std::cerr << exampleCommand << "\n";

}// end of CommandLineArgumentHandler::printBadCommandLineArgumentsError
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// CommandLineArgumentHandler::convertStringToDouble
//
/**
 *
 * Converts pString to a double and returns it via pValue.
 * 
 * @param pString   the string to be converted
 * @param pValue    the returned value...returns DBL_MAX if function returns non-zero
 * 
 * @return          0 on success, -1 on failure; on failure the value returned via pValue
 *                  is DBL_MAX
 * 
 */

int CommandLineArgumentHandler::convertStringToDouble(std::string pString, double *pValue)
{

    try{
        *pValue = std::stod(pString);
    }catch(...){
        *pValue = DBL_MAX;
        return(-1);
    }
    
    return(0);

}// end of ::convertStringToDouble
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// CommandLineArgumentHandler::convertStringToInt
//
/**
 *
 * Converts pString to an int and returns it via pValue.
 * 
 * @param pString   the string to be converted
 * @param pValue    the returned value...returns INT_MAX if function returns non-zero
 * 
 * @return          0 on success, -1 on failure; on failure the value returned via pValue
 *                  is INT_MAX
 * 
 */

int CommandLineArgumentHandler::convertStringToInt(std::string pString, int *pValue)
{

    try{
        *pValue = std::stoi(pString);
    }catch(...){
        *pValue = INT16_MAX;
        return(-1);
    }
    
    return(0);

}// end of ::convertStringToInt
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// CommandLineArgumentHandler::handleDoubleArgValue
//
/**
 * 
 * Searches the first element of pArgset for substring pArgTag. If found, then second element is
 * converted to a double and stored in pValue.
 * 
 * Returns 0 if the argument name contained the tag and the value was successfully converted.
 * Returns -1 if the value could not be converted.
 * Returns -2 if the argument name did not contain the tag.
 * 
 * @param pArgSet       a string vector containing an argument name as the first element
 *                      and the argument's value as the second element
 * @param pArgTag       the substring which is searched for in the argument name to determine
 *                      a match
 * @param pValue        a pointer to the variable in which the converted value is to be stored
 * 
 * @return              0 on success, -1 on failure, -2 if arg name does not contain the tag
 * 
 */

int CommandLineArgumentHandler::handleDoubleArgValue(std::vector<std::string> pArgSet,
                                                             std::string pArgTag, double *pValue)
{

    int status;

    int pos = pArgSet.at(0).find(pArgTag);

    if (pos != std::string::npos){
        status = convertStringToDouble(pArgSet.at(1), pValue);
        if(status == -1){ printBadCommandLineArgumentsError(); return(-1); }
        return(0);
    } else {
        return(-2);
    }

}// end of CommandLineArgumentHandler::handleDoubleArgValue
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// CommandLineArgumentHandler::handleIntArgValue
//
/**
 * 
 * Searches the first element of pArgset for pArgTag. If found, then second element is
 * converted to an int and stored in pValue.
 * 
 * Returns 0 if the argument name contained the tag and the value was successfully converted.
 * Returns -1 if the value could not be converted.
 * Returns -2 if the argument name did not contain the tag.
 * 
 * @param pArgSet       a string vector containing an argument name as the first element
 *                      and the argument's value as the second element
 * @param pArgTag       the substring which is searched for in the argument name to determine
 *                      a match
 * @param pValue        a pointer to the variable in which the converted value is to be stored
 * 
 * @return              0 on success, -1 on failure, -2 if arg name does not contain the tag
 * 
 */

int CommandLineArgumentHandler::handleIntArgValue(std::vector<std::string> pArgSet,
                                                             std::string pArgTag, int *pValue)
{

    int status;

    int pos = pArgSet.at(0).find(pArgTag);

    if (pos != std::string::npos){
        status = convertStringToInt(pArgSet.at(1), pValue);
        if(status == -1){ printBadCommandLineArgumentsError(); return(-1); }
        return(0);
    } else {
        return(-2);
    }

}// end of CommandLineArgumentHandler::handleIntArgValue
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// CommandLineArgumentHandler::handleBooleanArgValue
//
/**
 * 
 * Searches the first element of pArgset for pArgTag. If found, then second element is
 * converted to a boolean and stored in pValue.
 * 
 * If the second element is "true" then the value is set to TRUE while "false" will
 * set it to FALSE. If any other phrase is found, the value will be set to FALSE but
 * error -1 will be returned.
 * 
 * Returns 0 if the argument name contained the tag and the value was successfully converted.
 * Returns -1 if the value could not be converted.
 * Returns -2 if the argument name did not contain the tag.
 * 
 * @param pArgSet       a string vector containing an argument name as the first element
 *                      and the argument's value as the second element
 * @param pArgTag       the substring which is searched for in the argument name to determine
 *                      a match
 * @param pValue        a pointer to the variable in which the converted value is to be stored
 * 
 * @return              0 on success, -1 on failure, -2 if arg name does not contain the tag
 * 
 */

int CommandLineArgumentHandler::handleBooleanArgValue(std::vector<std::string> pArgSet,
                                                         std::string pArgTag, bool *pValue)
{

    int status, pos;

    pos = pArgSet.at(0).find(pArgTag);

    if (pos != std::string::npos){
        
        pos = pArgSet.at(1).find("true");
        if (pos != std::string::npos){ *pValue = true; return(0); }

        pos = pArgSet.at(1).find("false");
        if (pos != std::string::npos){ *pValue = false; return(0); }

        *pValue = false;
        printBadCommandLineArgumentsError();
        return(-1);

    } else {
        return(-2);
    }

}// end of CommandLineArgumentHandler::handleBooleanArgValue
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// CommandLineArgumentHandler::handleStringArgValue
//
/**
 * 
 * Searches the first element of pArgset for pArgTag. If found, then second element is saved to
 * pValue.
 * 
 * Returns 0 if the argument name contained the tag and the value was successfully converted.
 * Returns -2 if the argument name did not contain the tag.
 * 
 * @param pArgSet       a string vector containing an argument name as the first element
 *                      and the argument's value as the second element
 * @param pArgTag       the substring which is searched for in the argument name to determine
 *                      a match
 * @param pValue        a pointer to the variable in which the value is to be stored
 * 
 * @return              0 on success, -2 if arg name does not contain the tag
 * 
 */

int CommandLineArgumentHandler::handleStringArgValue(std::vector<std::string> pArgSet,
                                                 std::string pArgTag, std::string *pValue)
{

    int status;

    int pos = pArgSet.at(0).find(pArgTag);

    if (pos != std::string::npos){
        *pValue = pArgSet.at(1);
        return(0);
    } else {
        return(-2);
    }

}// end of CommandLineArgumentHandler::handleStringArgValue
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// CommandLineArgumentHandler::handleHelpOption
//
/**
 * 
 * Checks if pArg contains '-?', '-h', or '--help' and displays help info if it does.
 *
 * @param pArg  the string to be checked for -?, -h, or --help
 * 
 * @return      0 if pArg does not contain -?, -h, and --help; -1 if it does contain
 *              one of those phrases
 *  
 */ 

int CommandLineArgumentHandler::handleHelpOption(std::string pArg)
{


    int pos1 = pArg.find("?");
    int pos2 = pArg.find("-h");
    int pos3 = pArg.find("--help");

    if (pos1 == std::string::npos && pos2 == std::string::npos && pos3 == std::string::npos){
        return(0);
    }

    printf("\n");
    printf("Wacom I2C Tablet Daemon Help\n");
    printf("\n");
    printf("--Requirements--\n");
    printf("\n");
    printf(" enable the desired I2C bus\n");
    printf("\n");
    printf(" user must have read/write access to the I2C bus\n");
    printf("\n");
    printf(" execute 'sudo chmod +666 /dev/uinput' OR run this program as sudo\n");
    printf("    uinput will revert on reboot; use systemd script or similar to make permanent\n");
    printf("    systemd versions prior to 245(?) cannot set uinput permissions with udev rules\n");
    printf("\n");
    printf("--Command Line Options--\n");
    printf("\n");


    printf("--display_width=xxxx\n");
    printf("    The width of the display.");
    printf("    --display_width=1920\n");
    printf("\n");
    printf("--display_height=xxxx\n");
    printf("    The height of the display.");
    printf("    --display_height=1080\n");
    printf("\n");    
    printf("--x_scale=0.0921659\n");
    printf("    X direction scale to match pen range to display range.\n");
    printf("    (display width / usable width range on tablet)\n");
    printf("\n");
    printf("--y_scale=0.06912442\n");
    printf("    Y direction scale to match pen range to display range.\n");
    printf("    (display height / usable height range on tablet)\n");
    printf("\n");
    printf("--x_offset=0\n");
    printf("    X offset of tablet values to line up properly with display.\n");
    printf("\n");
    printf("--y_offset=0\n");
    printf("    Y offset of tablet values to line up properly with display.\n");
    printf("\n");
    printf("--transposeXY=true\n");
    printf("    true OR false\n");
    printf("    Switches the X & Y axis so pen direction matches display.\n");
    printf("\n");
    printf("--flip_x=false\n");
    printf("    true OR false\n");    
    printf("    Flips the X direction so pen direction matches display.\n");
    printf("    Note: if transposeXY=true, flipY to flipX\n");
    printf("\n");
    printf("--flip_y=true\n");
    printf("    true OR false\n");    
    printf("    Flips the Y direction so pen direction matches display.\n");
    printf("    Note: if transposeXY=true, flipX to flipY\n");
    printf("\n");
    printf("--touchdown_height=1\n");
    printf("    a value from 0 to 50; max value may vary depending on tablet model\n");
    printf("    Specifies the height above the tablet below which it is considered a 'touchdown' event.\n");
    printf("    Note that there are a few millimeters above the circuit layer which is all considered to be 0 height.\n");
    printf("\n");
    printf("--button_mode=0\n");
    printf("    0: a single left mouse button click will be generated when the pen touches down on the pad\n");
    printf("    1: the left mouse button will be held in the pressed state when the pen touches down on the pad\n");
    printf("        and released when the pen is held away from the pad\n");
    printf("\n");
    printf("--i2c_bus=i2c-1\n");
    printf("    i2c-1, i2c-4, etc.\n");
    printf("    Specifies which I2C bus to use for communcation with the tablet.\n");
    printf("\n");
    printf("--refresh_rate_MS=0\n");
    printf("    0 to 32,000\n");
    printf("    Specifies the time delay between each read of data from the tablet.\n");
    printf("\n");
    printf("--debug_mode=0\n");
    printf("    0: no debug info is displayed\n");
    printf("    1: tablet x,y and touchdown state are displayed on the terminal\n");
    printf("\n");

    return(-1);

}// end of CommandLineArgumentHandler::handleHelpOption
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// CommandLineArgumentHandler::parseArgument
//
/**
 * 
 * Parses the value from an argument having the format: --arg=value
 * 
 * The pArgumentString is checked against each possible argument and the associated variable is set
 * to the extracted value upon finding a match.
 * 
 * Returns 0 if the argument name contained a valid tag and the value was successfully converted.
 * Returns -1 if argument did contain a tag phrase but the value could not be converted.
 * Returns -2 if the argument name did not contain a valid tag phrase.
 * 
 * Examples:
 * 
 *      --x_scale=23.1           -> xScale will be set to 23.1
 *      --y_scale=46.2           -> yScale will be set to 46.2
 * 
 * @param pArgument     the CString containing the argument/value pair to be parsed
 * 
 * @return              0 on success, -1 on failure, -2 if argument phrase does not contain the tag
 * 
 */

int CommandLineArgumentHandler::parseArgument(char *pArgument)
{

    int status;

    std::string arg(pArgument);

    std::vector<std::string> splits = splitString(arg, '=');

    if(splits.size() < 2){
        printBadCommandLineArgumentsError();
        return(-1);
    }

    size_t pos;

    // if status is 0 (success) or -1 failure, exit with failure
    // otherwise keep searching for matching tag

    status = handleIntArgValue(splits, "display_width", &deviceInfo->displayWidth);
    if(status != -2){ return(status); }

    status = handleIntArgValue(splits, "display_height", &deviceInfo->displayHeight);
    if(status != -2){ return(status); }

    status = handleDoubleArgValue(splits, "x_scale", &deviceInfo->xScale);
    if(status != -2){ return(status); }

    status = handleDoubleArgValue(splits, "y_scale", &deviceInfo->yScale);
    if(status != -2){ return(status); }

    status = handleDoubleArgValue(splits, "y_offset", &deviceInfo->yOffset);  //debug mks -- flip this with next
    if(status != -2){ return(status); }

    status = handleDoubleArgValue(splits, "x_offset", &deviceInfo->xOffset);
    if(status != -2){ return(status); }

    status = handleBooleanArgValue(splits, "transposeXY", &deviceInfo->transposeXY);
    if(status != -2){ return(status); }

    status = handleBooleanArgValue(splits, "flip_x", &deviceInfo->flipX);
    if(status != -2){ return(status); }

    status = handleBooleanArgValue(splits, "flip_y", &deviceInfo->flipY);
    if(status != -2){ return(status); }

    status = handleIntArgValue(splits, "touchdown_height", &deviceInfo->touchdownHeight);
    if(status != -2){ return(status); }

    status = handleIntArgValue(splits, "button_mode", &deviceInfo->buttonMode);
    if(status != -2){ return(status); }

    status = handleStringArgValue(splits, "i2c_bus", &deviceInfo->i2cBusName);
    if(status == 0) { deviceInfo->i2cBusName = "/dev/" + deviceInfo->i2cBusName; return(status); }
    if(status == -1){ return(status); }

    status = handleIntArgValue(splits, "refresh_rate_MS", &deviceInfo->refreshRateMS);
    if (status == 0){ deviceInfo->refreshRateUS = deviceInfo->refreshRateMS * 1000; return(status); }
    if(status == -1){ return(status); }

    status = handleIntArgValue(splits, "debug_mode", &deviceInfo->debugMode);
    if(status != -2){ return(status); }

    return(0);

}// end of CommandLineArgumentHandler::parseArgument
//-------------------------------------------------------------------------------------------------
