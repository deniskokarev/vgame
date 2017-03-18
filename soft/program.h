/*
 * User-level API to program our mini-console
 * Just define your own Program class and create its global instance
 * Use either a typical event-handling model by overriding handleEvent()
 * or override execute() loop entirerly
 */

#include <cstdint>
#include "exec.h"
#include "Adafruit_PCD8544_HAL.h"

enum class Event: std::int8_t {
	EV_NONE = 0,
	EV_TIMER,
	EV_KEY_LEFT,
	EV_KEY_UP,
	EV_KEY_DOWN,
	EV_KEY_RIGHT,
	EV_KEY_ENTER
};

/* Event queue ring buffer */
class EventQueue {
protected:
	Event *q;
	int sz;
	int h, t;
public:
	EventQueue(Event *_q, int _sz);
	Event get();
	void put(Event e);
};

/* global events queue */
extern EventQueue events;

/* master program class to be redefined */
class Program {
protected:
	Adafruit_PCD8544_HAL display;
	Program();
public:
	/* enter STOP sleep mode until key pressed or time-out */
	void stopSleep(int sec);
	/* enter regular sleep mode until key pressed or time-out */
	void sleepSleep(int sec);
	/* if you want to use custom initialization don't forget to call master init() */
	virtual void init();
	/* you will have to define this function */
	virtual void handleEvent(Event event) = 0;
	/* you can redefine processing event loop entirerly */
	virtual void execute();
	/* entry point from C code */
	friend void ::exec();
};
