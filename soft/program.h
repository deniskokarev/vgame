/*
 * User-level API to program our mini-console
 * Just define your own Program class and create its global instance
 * Use either a typical event-handling model by overriding handleEvent()
 * or override execute() loop entirerly
 */

#include <cstdint>
#include "exec.h"
#include "AF_PCD8544_HAL.h"

enum class Event: std::int8_t {
	EV_NONE = 0,
	EV_CLOSE,
	EV_TIMER,
	EV_KEY_LEFT,
	EV_KEY_UP,
	EV_KEY_DOWN,
	EV_KEY_RIGHT,
	EV_KEY_ENTER,
	EV_CUSTOM
};

/* Event queue */
class EventQueue {
public:
	virtual Event get() = 0;
	virtual void put(Event e) = 0;
};

/* global events queue */
extern EventQueue *events;

class EventHandler {
public:
	/*
	 * handle one event and possibly transform it into another
	 */
	virtual Event handleEvent(Event event) = 0;
};

/* master program class to be redefined */
class Program: public EventHandler {
protected:
	AF_PCD8544_HAL display;
	int refresh;
	Program();
public:
	/* to be used by main program to pass the control to another program */
	static void setMainProgram(Program *p);
	/* if you want to use custom initialization don't forget to call master init() */
	virtual void init();
	/* enter STOP sleep mode until key pressed or time-out */
	void stopSleep(int sec);
	/* enter regular sleep mode until key pressed or time-out */
	void sleepSleep(int sec);
	/* you can redefine processing event loop entirerly */
	virtual void execute();
	/* when need to change sleep cycle */
	void setRefresh(int r);
	/* entry point from C code */
	friend void ::exec();
};

class Window: public EventHandler {
public:
	virtual void draw() = 0;
};

class WProgram: public Program {
protected:
	Window *mainWindow;
public:
	void setMainWindow(Window *w);
	virtual Event handleEvent(Event event) override;
};
