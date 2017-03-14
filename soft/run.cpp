#include "run.h"
#include "Adafruit_PCD8544_HAL.h"

/* SPI port number defined and initialized in HAL */
extern SPI_HandleTypeDef hspi1;

/* our pins defined and initialized in HAL */
static Adafruit_PCD8544_HAL_Pin dc {GPIOB, GPIO_PIN_7};  
static Adafruit_PCD8544_HAL_Pin cs {GPIOB, GPIO_PIN_6};
static Adafruit_PCD8544_HAL_Pin rst {GPIOA, GPIO_PIN_15};

Adafruit_PCD8544_HAL display = Adafruit_PCD8544_HAL(hspi1, dc, cs, rst);

// our main event loop to be invoked from HAL main.c
extern "C" void run() {
	display.begin();
	int color = BLACK;
	while (true) {
		display.fillScreen(color);
		display.display();
		HAL_Delay(1000);
		color ^= 1;
	}
}
