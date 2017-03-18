SHELL = bash

.DEFAULT_GOAL := all

# to use the master make file
AGCCBIN = /usr/local/gcc-arm-none-eabi-4_9-2015q2/bin/arm-none-eabi-
CUBE = ../../STM32Cube_FW_F3_V1.7.0
BOARD = STM32F3348-Discovery
CHIP = STM32F334x8

# LDSCRIPT may need to be crafted if your chip has a differemt mem size other than a discovery board
LDSCRIPT = STM32F334K8_FLASH.ld
include ./cube3.mk

CFLAGS += -IAdafruit-GFX-Library -Wall -std=c99 -Os -g
CXXFLAGS += -IAdafruit-GFX-Library -Wall -std=c++11 -Os -g
LDFLAGS += -g

# Binaries will be generated with this name (.elf, .bin, .hex, etc)
PROJ_NAME = vgame
SRC = $(wildcard Src/*.c)
INC = $(wildcard Inc/*.h) Adafruit_PCD8544_HAL.h program.h
OBJS = \
	$(SRC:%.c=%.o) \
	$(STARTUPOBJ) \
	cxx.o \
	Adafruit-GFX-Library/Adafruit_GFX.o \
	Adafruit-GFX-Library/glcdfont.o \
	Adafruit_PCD8544_HAL.o \
	program.o

Adafruit_PCD8544_HAL.o: $(INC)
program.o: $(INC)

all: $(PROJ_NAME).elf

# link with CC instead of CXX to watch out for accidental code bloating
$(PROJ_NAME).elf: $(OBJS) $(LIBBSP) $(LIBHAL)
	$(CC) $(LDFLAGS) -o $(@) $(OBJS) $(LIBBSP) $(LIBHAL)

clean: cube_clean
	rm -f *.o Src/*.o Adafruit-GFX-Library/*.o $(PROJ_NAME).elf $(PROJ_NAME).hex $(PROJ_NAME).bin

# Flash the MC
burn upload flash: $(PROJ_NAME).bin
	$(STFLASH) write $(PROJ_NAME).bin 0x8000000

# you need to have st-util running on different console
# we're automatically attaching gdb to it with `tar ext: 4242`
# then we flash and set breakpoint at main
debug: $(PROJ_NAME).elf gdb.x
	$(AGCCBIN)gdb -x gdb.x $(PROJ_NAME).elf

