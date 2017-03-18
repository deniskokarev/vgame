#include <cmath>
#include <random>
#include "Adafruit_PCD8544_HAL.h"
#include "run.h"

/* hspi1 SPI port number defined and initialized in HAL */
#include "spi.h"
#include "rtc.h"

extern "C" void SystemClock_Config(void);

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

template <typename T, int SZ> struct RingBuf {
	T q[SZ];
	int h, t;
	bool isEmpty() {
		return t != h;
	}
	T get() {
		if (t != h) {
			int rc = q[h];
			h = (h+1) % dim(q);
			return rc;
		} else {
			return T(0);
		}
	}
	void put(T e) {
		int nt = (t+1) % dim(q);
		if (nt != h) {
			q[t] = e;
			t = nt;
		} // otherwise just ignore
	}
};

struct Events : public RingBuf<uint8_t, 16> {
	enum {
		EV_NONE,
		EV_TIMER,
		EV_KEY_LEFT,
		EV_KEY_UP,
		EV_KEY_DOWN,
		EV_KEY_RIGHT,
		EV_KEY_ENTER
	};
	bool isKey(uint8_t e) {
		return e >= EV_KEY_LEFT && e <= EV_KEY_ENTER;
	}
};

Events events;

/*
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim == &htim6) {
		events.put(Events::EV_TIMER);
    }
}
*/

#define RTCHandle hrtc

extern "C" void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	switch (GPIO_Pin) {
	case GPIO_PIN_3:
		events.put(Events::EV_KEY_DOWN);
		break;
	case GPIO_PIN_4:
		events.put(Events::EV_KEY_LEFT);
		break;
	case GPIO_PIN_5:
		events.put(Events::EV_KEY_ENTER);
		break;
	case GPIO_PIN_6:
		events.put(Events::EV_KEY_RIGHT);
		break;
	case GPIO_PIN_7:
		events.put(Events::EV_KEY_UP);
		break;
	}
}

//HAL_RTCEx_WakeUpTimerIRQHandler

#if 0
extern "C" void HAL_RTCEx_WakeUpTimerIRQHandler(RTC_HandleTypeDef *hrtc) {
#else
extern "C" void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc) {
	//void RTC_WKUP_IRQHandler(void)
  events.put(Events::EV_TIMER);
}
#endif

//extern "C" void HAL_RTCEx_WakeUpTimerIRQHandler(RTC_HandleTypeDef *hrtc) {
//}

//extern "C" void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc) {
//	events.put(Events::EV_TIMER);
//}
	
// our main event loop to be invoked from HAL main.c
extern "C" void run() {
	display.begin();
#if 0
	HAL_SuspendTick();
	/* Request to enter SLEEP mode */
	HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
	/* Resume Tick interrupt if disabled prior to sleep mode entry*/
	HAL_ResumeTick();
#else
  /* Disable Wake-up timer */
  HAL_RTCEx_DeactivateWakeUpTimer(&RTCHandle);

  HAL_RTCEx_SetWakeUpTimer_IT(&RTCHandle, 2500*5, RTC_WAKEUPCLOCK_RTCCLK_DIV16);

  /* Enter Stop Mode */
  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

  /* Configures system clock after wake-up from STOP: enable HSI and PLL with HSI as source*/
  SystemClock_Config();
  
  /* Disable Wake-up timer */
  if(HAL_RTCEx_DeactivateWakeUpTimer(&RTCHandle) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler(); 
  }
#endif
	//DynamicPoint p[12];
	int y = events.get();
	while (true) {
		events.put(Events::EV_TIMER);
		if (events.get() == Events::EV_TIMER) {
			display.clearDisplay();
			for (auto x=0; x<display.width(); x++) {
				display.drawPixel(x, y, BLACK);
			}
			y++;
			y %= display.height();
			display.display();
		}
		HAL_Delay(100);
	}
}
