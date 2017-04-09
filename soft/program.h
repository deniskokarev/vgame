/**
 * User-level API to program our mini-console
 * Just define your own Program class and create its global instance
 * Use either a typical event-handling model by overriding handleEvent()
 * or override execute() loop entirerly
 @ @author Denis Kokarev
 */

#include <cstdint>
#include "exec.h"
#include "AF_PCD8544_HAL.h"
#include <type_traits>

/**
 * Events available on our hardware
 */
enum class Event: std::int8_t {
	EV_NONE = 0,
	EV_CLOSE,
	EV_TIMER,
	EV_KEY_LEFT,
	EV_KEY_UP,
	EV_KEY_DOWN,
	EV_KEY_RIGHT,
	EV_KEY_ENTER,
	EV_CUSTOM,
	EV_CUSTOM_1,
	EV_CUSTOM_2,
	EV_CUSTOM_3,
	EV_CUSTOM_4,
	EV_CUSTOM_5,
	EV_CUSTOM_6,
	EV_CUSTOM_7,
	EV_CUSTOM_8,
};

/**
 * in case if you want to refer to custom event codes in the form of EV_CUSTOM+1, EV_CUSTOM+2, etc
 */
constexpr Event operator+(const Event& x, int n) {
	return (Event)(std::underlying_type<Event>::type(x) + n);
}

/**
 * An abstract event queue to enqueue/dequeue events @see Event
 */
class EventQueue {
public:
	/**
	 * get next event from the queue 
	 * @return - available event or Event::EV_NONE
	 */
	virtual Event get() = 0;
	/**
	 * put event into the queue
	 * @param[in] e - any event
	 */
	virtual void put(Event e) = 0;
};

/**
 * Global events queue
 */
extern EventQueue *events;

/**
 * Base class to handle all events
 */
class EventHandler {
public:
	/**
	 * handle one event and possibly transform it into another
	 * @param[in] event - event to process
	 * @return - Event::EV_NONE (with Event::EV_CLOSE reserved for "focus" transfer in the future)
	 */
	virtual Event handleEvent(Event event) = 0;
};

/**
 * An abstract program class to work on our hardware.
 * You would need to define your own subclass and declare
 * just one global instance of it. This is enough to designate
 * your program as a primary event handler
 */
class Program: public EventHandler {
protected:
	AF_PCD8544_HAL display;
	int refresh;
	Program();
public:
	/*! to be used by current program to pass the control to another program */
	static void setMainProgram(Program *p);
	/*! you can use custom initialization, but don't forget to call master init() */
	virtual void init();
	/*! put CPU into STOP sleep mode until key pressed or time-out - best energy efficiency with slow wakeup */
	void stopSleep(int sec);
	/*! put CPU into regular sleep mode until key pressed or time-out - medium efficiency with fast wakeup */
	void sleepSleep(int sec);
	/**
	 * Simply get next event from the queue and invoke handleEvent() on it.
	 * if you want you can redefine processing event loop entirerly
	 */
	virtual void execute();
	/*! for how long to put CPU to sleep if no event available */
	void setRefresh(int r);
	/*! entry point from C code */
	friend void ::exec();
};

/**
 * A window-style event handler, which can draw on the screen
 */
class Window: public EventHandler {
public:
	/**
	 * display the window content. @see WProgram
	 */
	virtual void draw() = 0;
};

/**
 * A container that has multiple window(s) and dispatches events to the main window
 */
class WProgram: public Program {
protected:
	/*! the window that is in the "focus" - i.e. handles all events */
	Window *mainWindow;
public:
	/*! this way one window can pass the "focus" to another window */
	void setMainWindow(Window *w);
	/*! pass the event to the handleEvent() of the main window */
	virtual Event handleEvent(Event event) override;
};
