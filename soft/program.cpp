#include <cmath>
#include <random>
#include "Adafruit_PCD8544_HAL.h"
#include "exec.h"

/* hspi1 SPI port number defined and initialized in HAL */
#include "gpio.h"
#include "dma.h"
#include "spi.h"
#include "rtc.h"

#define dim(X)	(sizeof(X)/sizeof(X[0]))

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
};

Events events;

// defined in main.c, but not exported
extern "C" void SystemClock_Config(void);

class Program {
protected:
	static Program *program;
	
	static const Adafruit_PCD8544_HAL_Pin dc;
	static const Adafruit_PCD8544_HAL_Pin cs;
	static const Adafruit_PCD8544_HAL_Pin rst;
	Adafruit_PCD8544_HAL display;
	
	Program():display(hspi1, dc, cs, rst) {
		program = this;
	}

public:
	// takes about a sec to go into stopSleep
	void stopSleep(int sec) {
		/* Disable Wake-up timer */
		HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);
		HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 2500*sec, RTC_WAKEUPCLOCK_RTCCLK_DIV16);
		/* Enter Stop Mode */
		HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

		/* Configures system clock after wake-up from STOP: enable HSI and PLL with HSI as source*/
		SystemClock_Config();
		/* Disable Wake-up timer */
		if (HAL_RTCEx_DeactivateWakeUpTimer(&hrtc) != HAL_OK) {
			/* Initialization Error */
			Error_Handler(); 
		}
	}
	
	void sleepSleep(int sec) {
		/* Disable Wake-up timer */
		HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);
		HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 2500*sec, RTC_WAKEUPCLOCK_RTCCLK_DIV16);

		/*Suspend Tick increment to prevent wakeup by Systick interrupt. 
		  Otherwise the Systick interrupt will wake up the device within 1ms (HAL time base)*/
		HAL_SuspendTick();

		/* Request to enter SLEEP mode */
		HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);

		/* Resume Tick interrupt if disabled prior to sleep mode entry*/
		HAL_ResumeTick();

		/* Disable Wake-up timer */
		if (HAL_RTCEx_DeactivateWakeUpTimer(&hrtc) != HAL_OK) {
			/* Initialization Error */
			Error_Handler(); 
		}
	}
	
	virtual void init() {
		display.begin();
	}
	
	virtual void handleEvent(uint8_t event) = 0;

	static void execute() {
		Program::program->init();
		//Program::program->msleep(10000);

		while (true) {
			uint8_t event = events.get();
			if (event != Events::EV_NONE)
				Program::program->handleEvent(event);
			else
				Program::program->sleepSleep(1);
		}
	}
};

const Adafruit_PCD8544_HAL_Pin Program::dc {GPIOB, GPIO_PIN_7};
const Adafruit_PCD8544_HAL_Pin Program::cs {GPIOB, GPIO_PIN_6};
const Adafruit_PCD8544_HAL_Pin Program::rst {GPIOA, GPIO_PIN_15};

extern "C" void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	switch (GPIO_Pin) {
	case GPIO_PIN_3:
		events.put(Events::EV_KEY_DOWN);
		break;
	case GPIO_PIN_4:
		events.put(Events::EV_KEY_RIGHT);
		break;
	case GPIO_PIN_5:
		events.put(Events::EV_KEY_ENTER);
		break;
	case GPIO_PIN_6:
		events.put(Events::EV_KEY_LEFT);
		break;
	case GPIO_PIN_7:
		events.put(Events::EV_KEY_UP);
		break;
	}
}

extern "C" void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc) {
	events.put(Events::EV_TIMER);
}

extern "C" void exec() {
	Program::execute();
}

class MyProgram: public Program {
protected:
public:
	MyProgram():Program() {
	}
	virtual void handleEvent(uint8_t event) override {
		const char *msg;
		switch (event) {
		case Events::EV_KEY_LEFT:
			msg = "LEFT";
			break;
		case Events::EV_KEY_RIGHT:
			msg = "RIGHT";
			break;
		case Events::EV_KEY_UP:
			msg = "UP";
			break;
		case Events::EV_KEY_DOWN:
			msg = "DOWN";
			break;
		case Events::EV_KEY_ENTER:
			msg = "ENTER";
			break;
		default:
			msg = "SLEEPING...";
		}
		display.clearDisplay();
		display.setTextSize(1);
		display.setTextColor(BLACK);
		display.setCursor(0,0);
		display.print(msg);
		display.display();
	}
};

MyProgram mp;

Program *Program::program;
