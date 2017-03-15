#include <cmath>
#include <random>
#include "Adafruit_PCD8544_HAL.h"
#include "run.h"

/* hspi1 SPI port number defined and initialized in HAL */
#include "spi.h"

/* our pins defined and initialized in HAL */
static Adafruit_PCD8544_HAL_Pin dc {GPIOB, GPIO_PIN_7};  
static Adafruit_PCD8544_HAL_Pin cs {GPIOB, GPIO_PIN_6};
static Adafruit_PCD8544_HAL_Pin rst {GPIOA, GPIO_PIN_15};

static Adafruit_PCD8544_HAL display = Adafruit_PCD8544_HAL(hspi1, dc, cs, rst);

struct DynamicPoint {
	static std::uniform_real_distribution<double> unif;
	static std::default_random_engine re;
	int t;
	int x, y;
	int w, h;
	double speedX, speedY; // radian rotational speed for x,y
	DynamicPoint(int _w = LCDWIDTH, int _h = LCDHEIGHT):t(0),w(_w),h(_h) {
		x = (unif(re)+1.0)*w/2.0;
		y = (unif(re)+1.0)*h/2.0;
		// roughly +-1 degree
		speedX = unif(re)/90.0;
		speedY = unif(re)/90.0;
	}
	void next() {
		t++;
		x += sin(speedX * t);
		y += sin(speedY * t);
	}
};

std::uniform_real_distribution<double> DynamicPoint::unif(-1.0, 1.0);
std::default_random_engine DynamicPoint::re;

#define dim(X) ((sizeof(X)/sizeof(X[0])))
				
// our main event loop to be invoked from HAL main.c
extern "C" void run() {
	display.begin();
	//DynamicPoint p[12];
	int y = 0;
	while (true) {
		display.clearDisplay();
		for (auto x=0; x<display.width(); x++) {
			display.drawPixel(x, y, BLACK);
		}
		y++;
		y %= display.height();
		display.display();
		HAL_Delay(50);
	}
}
