extern "C" {
#include "lib/Config/DEV_Config.h"
#include "lib/GUI/GUI_BMPfile.h"
#include "lib/GUI/GUI_Paint.h"
#include "lib/e-Paper/EPD_IT8951.h"
}
#include "lib/Wacom/BasicTypes.h"
#include "lib/Wacom/WacomI2CHandler.h"

#include "CommandLineArgumentHandler.h"

#include <math.h>

#include <stdlib.h>     //exit()
#include <signal.h>     //signal()
#include <thread>

#define Enhance false
#define Four_Byte_Align false

#define USE_Factory_Test false

#define USE_Normal_Demo true

#define USE_Touch_Panel false

// 1 bit per pixel, which is 2 grayscale
#define BitsPerPixel_1 1
// 2 bit per pixel, which is 4 grayscale 
#define BitsPerPixel_2 2
// 4 bit per pixel, which is 16 grayscale
#define BitsPerPixel_4 4
// 8 bit per pixel, which is 256 grayscale, but will automatically reduce by hardware to 4bpp, which is 16 grayscale
#define BitsPerPixel_8 8

UWORD VCOM = 2050;

const int TABLET_I2C_ADDRESS = 9;

struct DeviceInfo deviceInfo;

WacomI2CHandler *tabletHandler;

struct TabletData tabletData;

IT8951_Dev_Info Dev_Info = {0, 0};
UWORD Panel_Width;
UWORD Panel_Height;
UWORD brush_Radius = 32;
UWORD Min_X = 0;
UWORD Max_X = 0;
UWORD Min_Y = 0;
UWORD Max_Y = 0;

UBYTE *Refresh_Frame_Buf2 = NULL;
UBYTE *Refresh_Frame_Buf3 = NULL;

UDOUBLE Init_Target_Memory_Addr;
int epd_mode = 0;	//0: no rotate, no mirror
					//1: no rotate, horizontal mirror, for 10.3inch
					//2: no totate, horizontal mirror, for 5.17inch
					//3: no rotate, no mirror, isColor, for 6inch color
					
void  Handler(int signo){
    
    Debug("\r\nHandler:exit\r\n");
    if(Refresh_Frame_Buf2 != NULL){
        free(Refresh_Frame_Buf2);
        Debug("free Refresh_Frame_Buf\r\n");
        Refresh_Frame_Buf2 = NULL;
    }
    if(Refresh_Frame_Buf3 != NULL){
        free(Refresh_Frame_Buf3);
        Debug("free Refresh_Frame_Buf\r\n");
        Refresh_Frame_Buf3 = NULL;
    }
	if(Dev_Info.Panel_W != 0){
		Debug("Going to sleep\r\n");
		//EPD_IT8951_Sleep();
	}
    DEV_Module_Exit();
    exit(0);
}

/******************************************************************************
function: Display_Image
parameter:
    Panel_Width: Width of the panel
    Panel_Height: Height of the panel
    Init_Target_Memory_Addr: Memory address of IT8951 target memory address
    BitsPerPixel: Bits Per Pixel, 2^BitsPerPixel = grayscale
******************************************************************************/
UBYTE Display_Image(char *filenm, UWORD Panel_Width, UWORD Panel_Height, UDOUBLE Target_Memory_Addr, UBYTE BitsPerPixel){
    UWORD WIDTH = Panel_Width;
    UWORD HEIGHT = Panel_Height;
    UBYTE *Local_Frame_Buf = NULL;

    UDOUBLE Imagesize;

    Imagesize = ((WIDTH * 1 % 8 == 0)? (WIDTH * 1 / 8 ): (WIDTH * 1 / 8 + 1)) * HEIGHT;
    if((Local_Frame_Buf = (UBYTE *)malloc(Imagesize)) == NULL){
        Debug("Failed to apply for picture memory...\r\n");
        return -1;
    }

    Paint_NewImage(Local_Frame_Buf, WIDTH, HEIGHT, 0, BLACK);
    Paint_SelectImage(Local_Frame_Buf);
    Paint_SetBitsPerPixel(BitsPerPixel);
    Paint_Clear(WHITE);

    char Path[256];
    sprintf(Path,filenm, WIDTH, HEIGHT);

    Debug("Reading file %s\r\n", Path);
    GUI_ReadBmp(Path, 0, 0);

    Debug("Drawing image\r\n");
    //you can draw your character and pattern on the image, for color definition of all BitsPerPixel, you can refer to GUI_Paint.h, 
    //Paint_DrawRectangle(50, 50, WIDTH/2, HEIGHT/2, 0x30, DOT_PIXEL_3X3, DRAW_FILL_EMPTY);
    //Paint_DrawCircle(WIDTH*3/4, HEIGHT/4, 100, 0xF0, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
    //Paint_DrawNum(WIDTH/4, HEIGHT/5, 709, &Font20, 0x30, 0xB0);

    switch(BitsPerPixel){
        case BitsPerPixel_8:{
            //Paint_DrawString_EN(10, 10, "8 bits per pixel 16 grayscale", &Font24, 0xF0, 0x00);
            EPD_IT8951_8bp_Refresh(Local_Frame_Buf, 0, 0, WIDTH,  HEIGHT, false, Target_Memory_Addr);
            break;
        }
        case BitsPerPixel_4:{
            //Paint_DrawString_EN(10, 10, "4 bits per pixel 16 grayscale", &Font24, 0xF0, 0x00);
            EPD_IT8951_4bp_Refresh(Local_Frame_Buf, 0, 0, WIDTH,  HEIGHT, false, Target_Memory_Addr,false);
            break;
        }
        case BitsPerPixel_2:{
            //Paint_DrawString_EN(10, 10, "2 bits per pixel 4 grayscale", &Font24, 0xC0, 0x00);
            EPD_IT8951_2bp_Refresh(Local_Frame_Buf, 0, 0, WIDTH,  HEIGHT, false, Target_Memory_Addr,false);
            break;
        }
        case BitsPerPixel_1:{
            //Paint_DrawString_EN(10, 10, "1 bit per pixel 2 grayscale", &Font24, 0x80, 0x00);
            EPD_IT8951_1bp_Refresh(Local_Frame_Buf, 0, 0, WIDTH,  HEIGHT, A2_Mode, Target_Memory_Addr,false);
            break;
        }
    }

    if(Local_Frame_Buf != NULL){
        free(Local_Frame_Buf);
        Local_Frame_Buf = NULL;
    }

    return 0;
}

int paintBackground() {
   Display_Image((char *)"/home/pi/Dev/READTYCH-DrawingDemo/pic/Notebook2.bmp", Panel_Width, Panel_Height, Init_Target_Memory_Addr, BitsPerPixel_1);
   return(0);
}

int paintInit(UWORD Radius) {
    UDOUBLE Imagesize;
   
    Debug("Paint init....\r\n");
        //malloc enough memory for 1bp picture first
    Imagesize = ((Panel_Width * 1 % 8 == 0)? (Panel_Width * 1 / 8 ): (Panel_Width * 1 / 8 + 1)) * Panel_Height;
    //Imagesize = ((dia * 1 % 8 == 0)? (dia * 1 / 8 ): (dia * 1 / 8 + 1)) * dia;
    if((Refresh_Frame_Buf2 = (UBYTE *)malloc(Imagesize)) == NULL){
        Debug("Failed to apply for picture memory...\r\n");
        return -1;
    }

    if((Refresh_Frame_Buf3 = (UBYTE *)malloc(Imagesize)) == NULL){
        Debug("Failed to apply for picture memory...\r\n");
        return -1;
    }

    Paint_NewImage(Refresh_Frame_Buf2, Panel_Width, Panel_Height, 0, BLACK);
    Paint_SelectImage(Refresh_Frame_Buf2);
    Paint_SetBitsPerPixel(1);

    Paint_Clear(WHITE);
    Min_X = Panel_Width;
    Min_Y = Panel_Height;
    Max_X = 0;
    Max_Y = 0;

    return(0);
}

int addBrushPoint(UWORD x, UWORD y, UWORD Radius) {
    Paint_DrawCircle(x, y, Radius, 0x00, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    UWORD xval;

    if (x - Radius < Min_X) {
        xval = x - Radius;
        Min_X = xval - (xval % 32);
        //Debug("Min %d %d\r\n", xval, Min_X);
    }
    if (x + Radius > Max_X) {
        xval = x + Radius;
        Max_X = (xval % 32 == 0)?xval:xval + 32 - (xval % 32);
        //Debug("Max %d %d\r\n", xval, Max_X);
    }

    if (y - Radius < Min_Y) {
        Min_Y = y - Radius;
    } 
    if (y + Radius > Max_Y) {
        Max_Y = y + Radius;
    } 

    return(0);
}


int subFrame(UBYTE *Buf, UWORD bufWidth, UBYTE *subBuf, UWORD x, UWORD y, UWORD width, UWORD height) {
    UBYTE val; 
    UWORD xByte;                        
    UWORD j,k;                          
    UWORD widthByte;

    widthByte = width * (Paint.BitsPerPixel) / 8;
    xByte = x * (Paint.BitsPerPixel) / 8;
    
    Debug("Sub Frame: %d %d (%d %d) %d %d\r\n", x, y, width, height, xByte, widthByte);
        
    for (k =0 ; k < height; k++) {
        for (j = 0; j < widthByte; j++) {
            val = Buf[xByte + j + (y+k)*bufWidth];
            //Debug("%u ",(unsigned int)val);
            subBuf[j+k*widthByte] = val;
        }
        //Debug("\r\n");
    }
        
    return(0);
}       
    
    
int paintBrush() {
    if ((Max_X == 0) || (Max_Y == 0)) return(0);

    UWORD x;
    UWORD y;        
    UWORD width;
    UWORD height;
    
    x = Min_X;
    y = Min_Y;
    width = Max_X - x;
    height = Max_Y - y;
    subFrame(Refresh_Frame_Buf2, Paint.WidthByte, Refresh_Frame_Buf3, x, y, width, height);
    Min_X = Panel_Width;
    Min_Y = Panel_Height;
    Max_X = 0;
    Max_Y = 0;
        
    //Debug("Painting Brush: %d %d %d %d\r\n", Min_X, Min_Y, width, height);
    EPD_IT8951_1bp_Refresh(Refresh_Frame_Buf3, x, y, width,  height, A2_Mode, Init_Target_Memory_Addr, true);
        
    return(0);
}

void updateDisplay() {
    while (1==1) {
       paintBrush();
       usleep(200000);
    }
}


//-------------------------------------------------------------------------------------------------
// ::handleTasks
//
/**
 * 
 * Handles all runtime tasks such as polling the tablet and updating the mouse status.
 * 
 */

int handleTasks() {
    bool resetDisplay = false;

    while(1){
        //Debug("Query pen data...\r\n");
        tabletHandler->queryPenData();

        if (tabletData.touchDown) {
                addBrushPoint(tabletData.transformedX, tabletData.transformedY, brush_Radius);
                if ((tabletData.transformedX < 100) && (resetDisplay == false)) {
                        resetDisplay = true;
                        //EPD_IT8951_Clear_Refresh(Dev_Info, Init_Target_Memory_Addr, INIT_Mode);
                        paintBackground();
                        paintInit(brush_Radius);
                } else if (tabletData.transformedX > 100) {
                        resetDisplay = false;
                }
        }

        if(deviceInfo.refreshRateUS > 0){
            usleep(deviceInfo.refreshRateUS);
        }

    }

    return 0;

}// end of ::HandleTasks


int main(int argc, char *argv[])
{
    //Exception handling:ctrl + c
    signal(SIGINT, Handler);
    int screen = 2;
    int status; 

    //Init the BCM2835 Device
    if(DEV_Module_Init()!=0){
        return -1;
    }

    CommandLineArgumentHandler commandLineArgumentHandler(&deviceInfo); 
    commandLineArgumentHandler.init();
    status = commandLineArgumentHandler.processArguments(argc, argv);
    if (status != 0){ return(status); }

/*
    sscanf(argv[1],"%d",&screen);

    if (argc > 2) {
        double temp;
        sscanf(argv[2],"%lf",&temp);
        VCOM = (UWORD)(fabs(temp)*1000);
    }

    if (argc > 3) {
	    sscanf(argv[3],"%d",&epd_mode);
    }
*/

    Debug("\r\nDisplay screen:%d\r\n",screen);
    Debug("VCOM value:%d\r\n", VCOM);
    Debug("Display mode:%d\r\n", epd_mode);
    Debug("Refresh Rate: %d\r\n", deviceInfo.refreshRateUS);

    swapSCREEN(screen);   //select the screen IO pins
    Dev_Info = EPD_IT8951_Init(VCOM);           // must reinitalize to work properly after swap I don't think all of the routines in this function are necessary here, it currently takes a few seconds for this funciton to run

    tabletHandler = new WacomI2CHandler(&deviceInfo, &tabletData);
    tabletHandler->init();

    tabletHandler->start(deviceInfo.i2cBusName.c_str(), TABLET_I2C_ADDRESS);

    tabletHandler->queryDeviceInfo();

    //get some important info from Dev_Info structure
    Panel_Width = Dev_Info.Panel_W;
    Panel_Height = Dev_Info.Panel_H;
    Init_Target_Memory_Addr = Dev_Info.Memory_Addr_L | (Dev_Info.Memory_Addr_H << 16);
    char* LUT_Version = (char*)"M841_TFA2812";
    Debug("LUT Mod Version = %s\r\n", LUT_Version);
    A2_Mode = 6;
    Debug("A2 Mode:%d\r\n", A2_Mode);

    //clear the screen
    EPD_IT8951_Clear_Refresh(Dev_Info, Init_Target_Memory_Addr, INIT_Mode);

    //display the background
    paintBackground();

    paintInit(brush_Radius);

    //process the loop
    handleTasks();

    //We recommended refresh the panel to white color before storing in the warehouse.
    EPD_IT8951_Clear_Refresh(Dev_Info, Init_Target_Memory_Addr, INIT_Mode);

    tabletHandler->stop();

    //EPD_IT8951_Standby();
    //EPD_IT8951_Sleep();

    //In case RPI is transmitting image in no hold mode, which requires at most 10s
    DEV_Delay_ms(5000);

    DEV_Module_Exit();
    return 0;
}

