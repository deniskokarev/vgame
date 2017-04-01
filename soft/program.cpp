/*
 * User-level API to progrm our mini-console
 */

#include "program.h"
#include "gpio.h"
#include "spi.h"	// has hspi1
#include "rtc.h"

/*
 * all of these must match the CubeMX initialized PINs
 * the SPI handle is expected to be named hspi1
 */
const static STM_HAL_Pin dc {GPIOB, GPIO_PIN_7};
const static STM_HAL_Pin cs {GPIOB, GPIO_PIN_6};
const static STM_HAL_Pin rst {GPIOA, GPIO_PIN_15};

/*** Events Queue *************************************/

/*
 * Our event queue is ring buffer
 */
class RingbufQueue: public EventQueue {
protected:
	Event *q;
	int sz;
	int h, t;
public:
	RingbufQueue(Event *_q, int _sz):q(_q),sz(_sz),h(0),t(0) {
	}
	virtual Event get() override {
		if (t != h) {
			Event rc = q[h];
			h = (h+1) % sz;
			return rc;
		} else {
			return Event::EV_NONE;
		}
	}
	virtual void put(Event e) override {
		int nt = (t+1) % sz;
		if (nt != h) {
			q[t] = e;
			t = nt;
		} // otherwise just ignore
	}
};

/* our events queue */
constexpr int EQ_SZ = 16;
static Event q[EQ_SZ];
RingbufQueue rbevents(q, EQ_SZ);
EventQueue *events = &rbevents;

/*** Program ******************************************/


/* the global singleton program for execution */
static Program *main_program;

void Program::setMainProgram(Program *p) {
	main_program = p;
}

/*
 * Merely declare your Program object to register it as main program
 * If you'll be declaring multiple programs the last constructed one
 * will become main_program
 * In that case, it would be a good idea to incapsulate all programs
 * into a single global object to specify which one to be created last
 */
Program::Program():display(hspi1, dc, cs, rst),refresh(1) {
	setMainProgram(this);
}

/* typical program initialization */
void Program::init() {
	display.begin();
}

/* need this fn defined in main.c, which is not exported */
extern "C" void SystemClock_Config(void);

/*
 * enter STOPSLEEP mode
 * to be awoken either by a key
 * or on timeout
 */
void Program::stopSleep(int sec) {
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

/*
 * enter SLEEP mode
 * to be awoken either by a key
 * or on timeout
 */
void Program::sleepSleep(int sec) {
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

/*
 * our typical execution loop
 */
void Program::execute() {
	init();

	while (true) {
		Event event = events->get();
		switch(event) {
		case Event::EV_NONE:
			sleepSleep(refresh);
			break;
		default:
			Event he = handleEvent(event);
			if (he == Event::EV_CLOSE)
				return;	// for example if main_program changed
		}
	}
}

/* when need to change sleep cycle */
void Program::setRefresh(int r) {
	refresh = r;
}

/*** WProgram *****************************************/

void WProgram::setMainWindow(Window *w) {
	mainWindow = w;
	mainWindow->draw();
}

/* we just have to redefine event handler */
Event WProgram::handleEvent(Event event) {
	return mainWindow->handleEvent(event);
}

/*** Hooks to HAL C code ******************************/

extern "C" {

	/* Buttons IRQ handler */
	void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
		switch (GPIO_Pin) {
		case GPIO_PIN_3:
			events->put(Event::EV_KEY_DOWN);
			break;
		case GPIO_PIN_4:
			events->put(Event::EV_KEY_RIGHT);
			break;
		case GPIO_PIN_5:
			events->put(Event::EV_KEY_ENTER);
			break;
		case GPIO_PIN_6:
			events->put(Event::EV_KEY_LEFT);
			break;
		case GPIO_PIN_7:
			events->put(Event::EV_KEY_UP);
			break;
		}
	}

	/* RTC wakeup IRQ handler */
	void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc) {
		events->put(Event::EV_TIMER);
	}

	/*
	 * Entry point from main.c
	 */
	void exec() {
		while(true)
			main_program->execute();
	}

}
