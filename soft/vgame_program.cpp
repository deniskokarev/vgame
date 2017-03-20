#include "program.h"
#include "game.h"

/*
 * A custom demo-program for our mini-console
 */

class MyProgram: public Program {
protected:
	static constexpr int gr = MAX_DIM;
	static constexpr int ysz = LCDHEIGHT/gr;
	static constexpr int yl = ysz*gr-1;
	static constexpr int xsz = ysz+1;
	static constexpr int xl = xsz*gr-1;
	static const unsigned char whiteChip[ysz-2][xsz-2];
	static const unsigned char blackChip[ysz-2][xsz-2];
	
	int cursorX;
	int cursorY;
	
	GAME_STATE board;
	
	void drawChip(int r, int c, CHIP_COLOR color) {
		int px = xsz * c + 1;
		int py = ysz * r + 1;
		if (color == COLOR_VACANT) {
			for (int y=0; y<ysz-2; y++)
				for (int x=0; x<xsz-2; x++)
					display.drawPixel(px+x, py+y, WHITE);
		} else {
			const unsigned char (&chip)[ysz-2][xsz-2] = (color == COLOR_WHITE)?whiteChip:blackChip;
			for (int y=0; y<ysz-2; y++)
				for (int x=0; x<xsz-2; x++)
					if (chip[y][x] != 0)
						display.drawPixel(px+x, py+y, BLACK);
		}
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
				cursorX(3),
				cursorY(3)
	{
		for (int i=0; i<gr; i++)
			for (int j=0; j<gr; j++)
				board[i][j] = COLOR_VACANT;
		board[gr/2-1][gr/2-1] = COLOR_WHITE;
		board[gr/2][gr/2] = COLOR_WHITE;
		board[gr/2-1][gr/2] = COLOR_BLACK;
		board[gr/2][gr/2-1] = COLOR_BLACK;
	}

	virtual void init() override {
		Program::init();
		display.clearDisplay();
		drawGrid();
		redrawBoard();
	}

	void redrawBoard() {
		for (int r=0; r<gr; r++)
			for (int c=0; c<gr; c++)
				drawChip(r, c, board[r][c]);
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
			cursorX+dx >= 0 && cursorX+dx < gr &&
			cursorY+dy >= 0 && cursorY+dy < gr)
		{
			drawCursor(cursorY, cursorX, WHITE);
			cursorX += dx;
			cursorY += dy;
		}
		redrawBoard();
	}
};

const unsigned char MyProgram::whiteChip[ysz-2][xsz-2] = {
	{0, 1, 1, 0},
	{1, 0, 0, 1},
	{0, 1, 1, 0},
};

const unsigned char MyProgram::blackChip[ysz-2][xsz-2] = {
	{0, 1, 1, 0},
	{1, 1, 1, 1},
	{0, 1, 1, 0},
};

/* just create a global instance of it to plug it in */
MyProgram mp;
