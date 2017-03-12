#
# Generic CubeF3 HAL library makefile to be included from your project Makefile
#
# the primary goal is to export CFLAGS and rules to make Cube libs,
# namely LIBBSP, LIBHAL, LIBCMSIS and LIBUSB
#
#

# Place where you've unpack your STM32 Cube F3
#CUBE = $(HOME)/work/arm/STM32Cube_FW_F3_V1.7.0
# must be defined in main makefile for the target board/chip
# BOARD must be one of $(CUBE)/Projects
#BOARD = STM32F3-Discovery
# must be one of definitions recognized by stm32f3xx.h 
#CHIP = STM32F303xC
TEMPL = $(CUBE)/Projects/$(BOARD)/Templates
BSPDIR = $(CUBE)/Drivers/BSP/$(BOARD)
HALDIR = $(CUBE)/Drivers/STM32F3xx_HAL_Driver
USBDIR = $(CUBE)/Middlewares/ST/STM32_USB_Device_Library
USBINCDIRS = $(wildcard $(USBDIR)/Class/*/Inc) $(USBDIR)/Core/Inc

# where is your no EABI ARM GCC compiler
#AGCCBIN = /usr/local/gcc-arm-none-eabi-4_9-2015q2/bin/arm-none-eabi-
CC = $(AGCCBIN)gcc
AS = $(AGCCBIN)as
AR = $(AGCCBIN)ar
LD = $(AGCCBIN)ld
RANLIB = $(AGCCBIN)ranlib
OBJCOPY = $(AGCCBIN)objcopy
STFLASH = st-flash

ARCH_FLAGS = -march=armv7e-m -mcpu=cortex-m4 -mlittle-endian -mthumb -mthumb-interwork -mfloat-abi=hard -mfpu=fpv4-sp-d16 -static

ifndef LDSCRIPT 
	LDSCRIPT = $(wildcard $(TEMPL)/TrueSTUDIO/*/*.ld)
endif
LDFLAGS = $(ARCH_FLAGS) -lm -lc --specs=nano.specs -Wl,--gc-sections -T$(LDSCRIPT)

CFLAGS = $(ARCH_FLAGS) -D$(CHIP) -DARM_MATH_CM4
CFLAGS += -nostartfiles -nostdlib
CFLAGS += -ffunction-sections -fdata-sections -fno-common
CFLAGS += -Wall 
CFLAGS += \
	-I. \
	-I./Inc \
	-I$(CUBE)/Drivers/CMSIS/Device/ST/STM32F3xx/Include \
	-I$(CUBE)/Drivers/CMSIS/Include \
	-I$(HALDIR)/Inc \
	-I$(BSPDIR) \
	$(foreach idir,$(USBINCDIRS),-I$(idir))

BSPSRC = $(wildcard $(BSPDIR)/*.c) $(wildcard $(BSPDIR)/../Components/*/*.c)
BSPOBJ := $(patsubst %.c,cubeobj/%.o,$(subst $(CUBE)/,,$(BSPSRC)))

# need to exclude template files from the mix, e.g: stm32f3xx_hal_msp_template.c
ALLHALSRC := $(wildcard $(HALDIR)/Src/*.c)
HALSRC := $(filter-out %_template.c, $(ALLHALSRC))
HALOBJ := $(patsubst %.c,cubeobj/%.o,$(subst $(CUBE)/,,$(HALSRC)))

USBSRC := $(wildcard $(USBDIR)/Core/Src/*.c)
USBOBJ := $(patsubst %.c,cubeobj/%.o,$(subst $(CUBE)/,,$(USBSRC)))

# making class usb templates slightly easier to use
# one could simply depend and link the necessary *.o files
# like $(UCDIR)/CDC/Src/usbd_cdc.o
USBCLASSSRC := $(wildcard $(USBDIR)/Class/*/Src/*.c)
USBCLASSOBJ := $(patsubst %.c,cubeobj/%.o,$(subst $(CUBE)/,,$(USBCLASSSRC)))
UCDIR = cubeobj/$(subst $(CUBE)/,,$(USBDIR))/Class

LIBBSP = libbsp.a
# vvv  this stupid thing uses _weak_ references extensively and therefore cannot be .a lib
LIBHAL = libhal.o
LIBCMSIS = $(CUBE)/Drivers/CMSIS/Lib/GCC/libarm_cortexM4lf_math.a
LIBUSB = libusb.a

cube_all: $(LIBBSP) $(LIBHAL) $(LIBCMSIS) $(LIBUSB)

cubeobj: fix_fups
	@for s in $(dir $(subst $(CUBE)/,,$(BSPSRC) $(HALSRC) $(USBSRC) $(USBCLASSSRC))); do \
		echo $$s; \
	done | \
	uniq | \
	while read d; do \
		mkdir -p $(@)/$$d; \
	done

fix_fups:
	@echo "also fixing known f.ckups..."
	@echo "creating !!!UPPER CASE!!! alias for usdb_cdc.h"
	@echo '#include <usbd_cdc.h>	/* somebody in ST still unaware of case-sensitive file systems */' >USBD_CDC.h
	touch $(@)

$(HALOBJ) $(BSPOBJ) $(USBOBJ) $(USBCLASSOBJ): cubeobj/%.o: $(CUBE)/%.c
	$(CC) $(CFLAGS) -c $(<) -o $(@)

$(LIBBSP): $(BSPOBJ)
	$(AR) -cr $(@) $(?)
	$(RANLIB) $(@)

# this stupid thing uses _weak_ references extensively and therefore cannot be .a lib
$(LIBHAL): $(HALOBJ)
	$(LD) -r -o $(@) $(HALOBJ)

$(LIBUSB): $(USBOBJ)
	$(AR) -cr $(@) $(?)
	$(RANLIB) $(@)

# system obj may be safely borrowed too
SYSTEMSRCC = $(wildcard $(TEMPL)/Src/system*.c)
SYSTEMOBJ = $(patsubst %.c,cubeobj/%.o,$(subst $(TEMPL)/Src/,,$(SYSTEMSRCC)))
$(SYSTEMOBJ): $(SYSTEMSRCC)
	$(CC) $(CFLAGS) -o $(@) -c $(<)

STARTUPSRCS = $(wildcard $(TEMPL)/TrueSTUDIO/startup_*.s)
STARTUPOBJ = $(patsubst %.s,cubeobj/%.o,$(subst $(TEMPL)/TrueSTUDIO/,,$(STARTUPSRCS)))
$(STARTUPOBJ): $(STARTUPSRCS)
	$(AS) -o $(@) -c $(<)

$(BSPOBJ) $(BSPCOMPOBJ) $(HALOBJ) $(USBOBJ) $(USBCLASSOBJ) $(SYSTEMOBJ) $(STARTUPOBJ): cubeobj

cube_clean:
	rm -f $(LIBBSP) $(LIBHAL) $(LIBUSB)
	rm -rf cubeobj
	rm -f fix_fups USBD_CDC.h
	rm -f gdb.x

.SUFFIXES: .elf .bin .hex

.elf.hex:
	$(OBJCOPY) -O ihex $(<) $(@)

.elf.bin:
	$(OBJCOPY) -O binary $(<) $(@)

.PHONY: gdb.x

gdb.x:
	if ! pgrep 'st-util'; then \
		echo '!!! you must start st-util first !!!'; \
		false; \
	fi
	@(echo "target ext: 4242"; echo "load"; echo "break main"; echo "continue") >$(@)
