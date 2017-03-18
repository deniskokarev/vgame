/*********************************************************************
This is a library for our Monochrome Nokia 5110 LCD Displays

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/338

These displays use SPI to communicate, 4 or 5 pins are required to  
interface

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.  
BSD license, check license.txt for more information
All text above, and the splash screen below must be included in any redistribution
*********************************************************************/

#include <cstdlib>

#include "Adafruit_PCD8544_HAL.h"
#include "stm32f3xx_hal.h"

#define	LOW		GPIO_PIN_RESET
#define	HIGH	GPIO_PIN_SET


static void digitalWrite(const Adafruit_PCD8544_HAL_Pin &pin, GPIO_PinState val) {
	HAL_GPIO_WritePin(pin.base, pin.pin, val);
}

#ifndef _BV
  #define _BV(bit) (1<<(bit))
#endif

Adafruit_PCD8544_HAL::Adafruit_PCD8544_HAL(SPI_HandleTypeDef &hspi,
										   const Adafruit_PCD8544_HAL_Pin &dc,
										   const Adafruit_PCD8544_HAL_Pin &cs,
										   const Adafruit_PCD8544_HAL_Pin &rst
										   ):
	Adafruit_GFX(LCDWIDTH, LCDHEIGHT),
	_hspi(hspi),
	_dc(dc),
	_cs(cs),
	_rst(rst)
{
}


// the most basic function, set a single pixel
void Adafruit_PCD8544_HAL::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height))
    return;

  int16_t t;
  switch(rotation){
    case 1:
      t = x;
      x = y;
      y =  LCDHEIGHT - 1 - t;
      break;
    case 2:
      x = LCDWIDTH - 1 - x;
      y = LCDHEIGHT - 1 - y;
      break;
    case 3:
      t = x;
      x = LCDWIDTH - 1 - y;
      y = t;
      break;
  }

  if ((x < 0) || (x >= LCDWIDTH) || (y < 0) || (y >= LCDHEIGHT))
    return;

  // x is which column
  if (color) 
    pcd8544_buffer[x+ (y/8)*LCDWIDTH] |= _BV(y%8);  
  else
    pcd8544_buffer[x+ (y/8)*LCDWIDTH] &= ~_BV(y%8); 

}


// the most basic function, get a single pixel
uint8_t Adafruit_PCD8544_HAL::getPixel(int8_t x, int8_t y) {
  if ((x < 0) || (x >= LCDWIDTH) || (y < 0) || (y >= LCDHEIGHT))
    return 0;

  return (pcd8544_buffer[x+ (y/8)*LCDWIDTH] >> (y%8)) & 0x1;  
}


void Adafruit_PCD8544_HAL::begin(uint8_t contrast, uint8_t bias) {
  digitalWrite(_rst, LOW);
  HAL_Delay(250);
  digitalWrite(_rst, HIGH);
  HAL_Delay(250);

  digitalWrite(_cs, LOW);
  digitalWrite(_dc, LOW);
  mode = COMMAND;
  
  // get into the EXTENDED mode!
  command(PCD8544_FUNCTIONSET | PCD8544_EXTENDEDINSTRUCTION );

  // LCD bias select (4 is optimal?)
  command(PCD8544_SETBIAS | bias);

  // set VOP
  if (contrast > 0x7f)
    contrast = 0x7f;

  command( PCD8544_SETVOP | contrast); // Experimentally determined


  // normal mode
  command(PCD8544_FUNCTIONSET);

  // Set display to Normal
  command(PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYNORMAL);
  digitalWrite(_cs, HIGH);
}


inline void Adafruit_PCD8544_HAL::data(uint8_t *p, uint16_t sz) {
    // Hardware SPI write.
	if (mode != DATA) {
		while(HAL_SPI_GetState(&_hspi) != HAL_SPI_STATE_READY) {
		}
		digitalWrite(_dc, HIGH);
		mode = DATA;
	}
	HAL_SPI_Transmit_DMA(&_hspi, p, sz);
	while(HAL_SPI_GetState(&_hspi) != HAL_SPI_STATE_READY) {
	}
}

void Adafruit_PCD8544_HAL::command(uint8_t c) {
	if (mode != COMMAND) {
		while(HAL_SPI_GetState(&_hspi) != HAL_SPI_STATE_READY) {
		}
		digitalWrite(_dc, LOW);
		mode = COMMAND;
	}
	HAL_SPI_Transmit(&_hspi, &c, 1, 0);
}

void Adafruit_PCD8544_HAL::setContrast(uint8_t val) {
  if (val > 0x7f) {
    val = 0x7f;
  }
  command(PCD8544_FUNCTIONSET | PCD8544_EXTENDEDINSTRUCTION );
  command( PCD8544_SETVOP | val); 
  command(PCD8544_FUNCTIONSET);
 }



void Adafruit_PCD8544_HAL::display(void) {
  
	digitalWrite(_cs, LOW);
  
    command(PCD8544_SETYADDR | 0);
    command(PCD8544_SETXADDR | 0);

	data(pcd8544_buffer, sizeof(pcd8544_buffer));
	
	digitalWrite(_cs, HIGH);
}

// clear everything
void Adafruit_PCD8544_HAL::clearDisplay(void) {
  memset(pcd8544_buffer, 0, LCDWIDTH*LCDHEIGHT/8);
  cursor_y = cursor_x = 0;
}
