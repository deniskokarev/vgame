#include <cstdint>
#include "exec.h"
#include "Adafruit_PCD8544_HAL.h"

enum class Event {
	EV_NONE,
	EV_TIMER,
	EV_KEY_LEFT,
	EV_KEY_UP,
	EV_KEY_DOWN,
	EV_KEY_RIGHT,
	EV_KEY_ENTER
};

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

extern EventQueue events;

class Program {
protected:
	Adafruit_PCD8544_HAL display;
	Program();
public:
	void stopSleep(int sec);
	void sleepSleep(int sec);
	virtual void init();
	virtual void handleEvent(Event event) = 0;
	virtual void execute();
	friend void ::exec();
};
