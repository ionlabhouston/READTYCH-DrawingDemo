DIR_Config   = ./lib/Config
DIR_EPD      = ./lib/e-Paper
DIR_FONTS    = ./lib/Fonts
DIR_GUI      = ./lib/GUI
DIR_Examples = ./examples
DIR_Wacom = ./lib/Wacom

DIR_BIN      = ./bin

OBJ_CPP = $(wildcard ${DIR_Config}/*.cpp ${DIR_EPD}/*.cpp ${DIR_FONTS}/*.cpp ${DIR_GUI}/*.cpp *.cpp ${DIR_Wacom}/*.cpp )
OBJ_C = $(wildcard ${DIR_Config}/*.c ${DIR_EPD}/*.c ${DIR_FONTS}/*.c ${DIR_GUI}/*.c ${DIR_Examples}/*.c )
OBJ_O = $(patsubst %.c,${DIR_BIN}/%.o,$(notdir ${OBJ_C}))
OBJ_OPP = $(patsubst %.cpp,${DIR_BIN}/%.o,$(notdir ${OBJ_CPP}))

TARGET = epd

CC = gcc

MSG = -g -O0 -Wall
DEBUG = -D USE_DEBUG
STD = -std=gnu99

CFLAGS += $(MSG) $(DEBUG) $(STD)

LIB = -lbcm2835 -lm -lrt -lpthread -lstdc++

$(shell mkdir -p $(DIR_BIN))

${TARGET}:${OBJ_O} ${OBJ_OPP}
	$(CC) $(CFLAGS) $(OBJ_O) $(OBJ_OPP) -o $@ $(LIB) 

${DIR_BIN}/%.o:%.cpp
	$(CC) $(CFLAGS) -c  $< -o $@

${DIR_BIN}/%.o:$(DIR_Wacom)/%.cpp
	$(CC) $(CFLAGS) -c  $< -o $@

${DIR_BIN}/%.o:$(DIR_Config)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@ 

${DIR_BIN}/%.o:$(DIR_EPD)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@ 
	
${DIR_BIN}/%.o:$(DIR_FONTS)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@ 
	
${DIR_BIN}/%.o:$(DIR_GUI)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@ 
	
${DIR_BIN}/%.o:$(DIR_Examples)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@ 

clean :
	rm $(DIR_BIN)/*.* 
	rm $(TARGET) 

