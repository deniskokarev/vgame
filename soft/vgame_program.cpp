#include "program.h"
//#include <algorithm>

/*
 * A custom demo-program for our mini-console
 */

const unsigned char whiteChip[5][6] = {
	{0, 0, 0, 0, 0, 0},
	{0, 0, 1, 1, 0, 0},
	{0, 1, 0, 0, 1, 0},
	{0, 0, 1, 1, 0, 0},
	{0, 0, 0, 0, 0, 0},
};

const unsigned char blackChip[5][6] = {
	{0, 0, 0, 0, 0, 0},
	{0, 0, 1, 1, 0, 0},
	{0, 1, 1, 1, 1, 0},
	{0, 0, 1, 1, 0, 0},
	{0, 0, 0, 0, 0, 0},
};

class MyProgram: public Program {
protected:
	const int gr = 8;
	const int ysz;
	const int yl;
	const int xsz;
	const int xl;

	void drawChip(int r, int c, int color) {
		const unsigned char (&chip)[5][6] = (color == WHITE)?whiteChip:blackChip;
		int px = xsz * c;
		int py = ysz * r;
		for (int y=0; y<ysz-1; y++)
			for (int x=0; x<xsz-1; x++)
				if (chip[y][x] != 0)
					display.drawPixel(px+x, py+y, BLACK);
	}
										  
	void drawGrid() {
		for (int n=0; n<gr-1; n++) {
			int px = ysz*n+ysz-1;
			int py = xsz*n+xsz-1;
			display.drawLine(0, px, xl, px, BLACK);
			display.drawLine(py, 0, py, yl, BLACK);
		}
	}
	
public:
	MyProgram():Program(),
				ysz(display.height()/gr),
				yl(ysz*gr-1),
				xsz(ysz+1),
				xl(xsz*gr-1) {}

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
		//display.print(msg);
		drawGrid();
		int clr = WHITE;
		for (int r=0; r<8; r++) {
			for (int c=0; c<8; c++) {
				drawChip(r, c, clr);
				clr ^= 1;
			}
		}
		display.display();
	}
};

/* just create a global instance of it to plug it in */
MyProgram mp;
