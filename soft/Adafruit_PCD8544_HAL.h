/*********************************************************************
This is a library for our Monochrome Nokia 5110 LCD Displays

Augmented to work with STM32 HAL library

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/338

These displays use SPI to communicate, 4 or 5 pins are required to  
interface

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.  
Augmented to work with HAL by Denis Kokarev
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution
*********************************************************************/
#ifndef _ADAFRUIT_PCD8544_HAL_H
#define _ADAFRUIT_PCD8544_HAL_H

#include "Adafruit_GFX.h"
#include "stm32f3xx_hal.h"

#define BLACK 1
#define WHITE 0

#define LCDWIDTH 84
#define LCDHEIGHT 48

#define PCD8544_POWERDOWN 0x04
#define PCD8544_ENTRYMODE 0x02
#define PCD8544_EXTENDEDINSTRUCTION 0x01

#define PCD8544_DISPLAYBLANK 0x0
#define PCD8544_DISPLAYNORMAL 0x4
#define PCD8544_DISPLAYALLON 0x1
#define PCD8544_DISPLAYINVERTED 0x5

// H = 0
#define PCD8544_FUNCTIONSET 0x20
#define PCD8544_DISPLAYCONTROL 0x08
#define PCD8544_SETYADDR 0x40
#define PCD8544_SETXADDR 0x80

// H = 1
#define PCD8544_SETTEMP 0x04
#define PCD8544_SETBIAS 0x10
#define PCD8544_SETVOP 0x80

struct Adafruit_PCD8544_HAL_Pin {
	GPIO_TypeDef* base;
	uint16_t pin;
};

class Adafruit_PCD8544_HAL : public Adafruit_GFX {
 public:
  // SPI and all pins should already be initialized at the moment of screen construction 
  Adafruit_PCD8544_HAL(SPI_HandleTypeDef &hspi,
					   const Adafruit_PCD8544_HAL_Pin &dc,
					   const Adafruit_PCD8544_HAL_Pin &cs,
					   const Adafruit_PCD8544_HAL_Pin &rst
					   );

  void begin(uint8_t contrast = 60, uint8_t bias = 0x04);
  
  void command(uint8_t c);
  void data(uint8_t *p, uint16_t sz);
  
  void setContrast(uint8_t val);
  void clearDisplay(void);
  void display();
  
  void drawPixel(int16_t x, int16_t y, uint16_t color);
  uint8_t getPixel(int8_t x, int8_t y);

 private:
  enum {
	  COMMAND = 0,
	  DATA = 1
  };
  SPI_HandleTypeDef &_hspi;
  const Adafruit_PCD8544_HAL_Pin &_dc;
  const Adafruit_PCD8544_HAL_Pin &_cs;
  const Adafruit_PCD8544_HAL_Pin &_rst;
  uint8_t pcd8544_buffer[LCDWIDTH * LCDHEIGHT / 8];
  uint8_t mode;
};

#endif
