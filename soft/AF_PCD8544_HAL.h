/**
 * @file
 * @brief Nokia LCD subclass for Adafruit GFX lib
 *
 * This is a library for our Monochrome Nokia 5110 LCD Displays
 * @author Limor Fried/Ladyada
 * @author Denis Kokarev
 */
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
BSD license, check license.txt for more information
Augmented to work with STM32 HAL by Denis Kokarev
*********************************************************************/
#ifndef _AF_PCD8544_HAL_H
#define _AF_PCD8544_HAL_H

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

/**
 * @brief Combining STM32 pin into a single structure
 */
struct STM_HAL_Pin {
	GPIO_TypeDef* base;
	uint16_t pin;
};

/**
 * @brief Nokia LCD display implementation
 *
 * A subclass of a generic Arduino-based Adafruit GFX library
 * to use Nokia LCD display on STM32 HAL platform
 * @author Limor Fried/Ladyada 
 * @author Denis Kokarev
 */
class AF_PCD8544_HAL : public Adafruit_GFX {
 public:
	/**
	 * @brief Construct the screen but don't send any commands to it yet.
	 *
	 * SPI and all pins should be initialized by the time of begin()
	 */
	AF_PCD8544_HAL(SPI_HandleTypeDef &hspi,
				   const STM_HAL_Pin &dc,
				   const STM_HAL_Pin &cs,
				   const STM_HAL_Pin &rst
				   );

	/**
	 * @brief send initialization sequence to the screen
	 */
	void begin(uint8_t contrast = 60, uint8_t bias = 0x04);
	/**
	 * @brief send contrast command to the screen
	 */
	void setContrast(uint8_t val);
	/**
	 * @brief clear display buffer (but don't refresh the screen)
	 */ 
	void clearDisplay(void);
	/**
	 * @brief push the buffer content to the screen
	 *
	 * The primary function to copy the entire buffer to the screen.
	 * Use it every time you want to update screen content after series of
	 * drawing functions, such as drawLine(), print(), etc
	 */
	void display();
	/**
	 * @brief check the pixel in the buffer
	 */
	uint8_t getPixel(int8_t x, int8_t y);
	/**
	 * @brief draw one pixel in the buffer, which unlocks potential of all other Adafruit_GFX functions
	 */
	void drawPixel(int16_t x, int16_t y, uint16_t color) override;
	/**
	 * @brief send command code to the screen
	 */
	void command(uint8_t c);
	/**
	 * @brief send a data block to the screen
	 */
	void data(uint8_t *p, uint16_t sz);
  
 protected:
	SPI_HandleTypeDef &_hspi;	///< SPI device handler
	const STM_HAL_Pin &_dc;		///< Data/Command pin
	const STM_HAL_Pin &_cs;		///< SPI chip select pin
	const STM_HAL_Pin &_rst;	///< Reset pin
	uint8_t pcd8544_buffer[LCDWIDTH * LCDHEIGHT / 8]; ///< screen buffer to hold all pixels
	/**
	 * @brief what did we push via SPI last time
	 * 
	 * This flag is necessary to prevent _dc pin change before all commands or all data
	 * finish transmission
	 * @see command()
	 * @see data()
	 */
	uint8_t mode;
};

#endif
