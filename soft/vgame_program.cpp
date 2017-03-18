#include "program.h"

class MyProgram: public Program {
protected:
public:
	MyProgram():Program() {
	}
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

MyProgram mp;
