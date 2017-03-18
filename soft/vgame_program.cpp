#include "program.h"

/*
 * A custom demo-program for our mini-console
 */

class MyProgram: public Program {
protected:
public:
	/* we just have to redefine event handler */
	virtual void handleEvent(Event event) override {
		const char *msg;
		switch (event) {
		case Event::EV_KEY_LEFT:
			msg = "LEFT";
			break;
		case Event::EV_KEY_RIGHT:
			msg = "RIGHT";
			break;
		case Event::EV_KEY_UP:
			msg = "UP";
			break;
		case Event::EV_KEY_DOWN:
			msg = "DOWN";
			break;
		case Event::EV_KEY_ENTER:
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

/* just create a global instance of it to plug it in */
MyProgram mp;
