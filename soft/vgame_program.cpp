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

static constexpr int gr = 8;

class MyProgram: public Program {
protected:
	const int ysz;
	const int yl;
	const int xsz;
	const int xl;

	int cursorX;
	int cursorY;
	
	int board[gr][gr];
	
	void drawNoChip(int r, int c) {
		int px = xsz * c;
		int py = ysz * r;
		for (int y=0; y<ysz-1; y++)
			for (int x=0; x<xsz-1; x++)
				display.drawPixel(px+x, py+y, WHITE);
	}

	void drawChip(int r, int c, int color) {
		const unsigned char (&chip)[5][6] = (color == WHITE)?whiteChip:blackChip;
		int px = xsz * c;
		int py = ysz * r;
		for (int y=0; y<ysz-1; y++)
			for (int x=0; x<xsz-1; x++)
				if (chip[y][x] != 0)
					display.drawPixel(px+x, py+y, BLACK);
	}
										  
	void drawCursor(int r, int c, int color) {
		int px = xsz * c;
		int py = ysz * r;
		display.drawLine(px, py, px+xsz-2, py, color);
		display.drawLine(px, py+ysz-2, px+xsz-2, py+ysz-2, color);
		display.drawLine(px, py, px, py+ysz-2, color);
		display.drawLine(px+xsz-2, py, px+xsz-2, py+ysz-2, color);
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
				xl(xsz*gr-1),
				cursorX(3),
				cursorY(3)
	{
		for (int i=0; i<gr; i++)
			for (int j=0; j<gr; j++)
				board[i][j] = 0;
		board[3][3] = 1;
		board[4][4] = 1;
		board[3][4] = 2;
		board[4][3] = 2;
	}

	virtual void init() override {
		Program::init();
		display.clearDisplay();
		drawGrid();
		redrawBoard();
	}

	void redrawBoard() {
		for (int r=0; r<8; r++) {
			for (int c=0; c<8; c++) {
				if (board[r][c] == 1) {
					drawChip(r, c, WHITE);
				} else if (board[r][c] == 2) {
					drawChip(r, c, BLACK);
				} else {
					drawNoChip(r, c);
				}
			}
		}
		drawCursor(cursorY, cursorX, BLACK);
		display.display();
	}
	
	/* we just have to redefine event handler */
	virtual void handleEvent(Event event) override {
		int dx=0, dy=0;
		switch (event) {
		case Event::EV_KEY_LEFT:
			dx = -1;
			break;
		case Event::EV_KEY_RIGHT:
			dx = 1;
			break;
		case Event::EV_KEY_UP:
			dy = -1;
			break;
		case Event::EV_KEY_DOWN:
			dy = 1;
			break;
		case Event::EV_KEY_ENTER:
			board[cursorY][cursorX]++;
			board[cursorY][cursorX] %= 3;
			break;
		default:
			return;
		}
		if ((dx != 0 || dy != 0) &&
			cursorX+dx >= 0 && cursorX+dx < 8 &&
			cursorY+dy >= 0 && cursorY+dy < 8)
		{
			drawCursor(cursorY, cursorX, WHITE);
			cursorX += dx;
			cursorY += dy;
		}
		redrawBoard();
	}
};

/* just create a global instance of it to plug it in */
MyProgram mp;
