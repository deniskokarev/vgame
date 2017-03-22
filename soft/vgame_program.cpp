#include "program.h"

extern "C" {
#include "game.h"
}

#if 0
static char *
lltoan(char *out, long long i, int n) {
	char *p = out;
	static char s[] = "0123456789";
	int v = n;
	while (v > 0) {
		int d = i % 10;
		i /= 10;
		p = s + d;
		out[--v] = *p;
	}
	return(out + n);
}
#endif

/*
 * A custom demo-program for our mini-console
 */

class MyProgram: public Program {
protected:
	class MyWindow: public EventHandler {
	protected:
		static MyProgram &program;
	public:
		virtual void draw() = 0;
	};

protected:
	class StartWindow: public MyWindow {
	protected:
		char *label; // cannot be const due to stupid Adafruit lib
		Adafruit_GFX_Button startButton;
	public:
		StartWindow():MyWindow(),label((char*)"START") {
			int16_t x, y;
			uint16_t w, h;
			program.display.getTextBounds(label, 0, 0, &x, &y, &w, &h);
			startButton.initButton(&program.display,
								   program.display.width()/2,
								   program.display.height()/4*3,
								   w+8,
								   h+4,
								   1,
								   0,
								   BLACK,
								   label,
								   1);
		}
		virtual Event handleEvent(Event event) override {
			switch(event) {
			case Event::EV_KEY_ENTER:
				startButton.drawButton(true);
				program.display.display();
				HAL_Delay(300);
				program.setMainWindow(&program.gameWindow);
				break;
			default:
				break;
			}
			return Event::EV_NONE;
		}
		virtual void draw() override {
			program.display.print("Reversy v0.8");
			startButton.drawButton();
			program.display.display();
		};
	};

	class GameWindow: public MyWindow {
	protected:
		void drawChip(int r, int c, CHIP_COLOR color) {
			int px = xsz * c + 1;
			int py = ysz * r + 1;
			if (color == COLOR_VACANT) {
				for (int y=0; y<ysz-2; y++)
					for (int x=0; x<xsz-2; x++)
						program.display.drawPixel(px+x, py+y, WHITE);
			} else {
				const unsigned char (&chip)[ysz-2][xsz-2] = (color == COLOR_WHITE)?whiteChip:blackChip;
				for (int y=0; y<ysz-2; y++)
					for (int x=0; x<xsz-2; x++)
						if (chip[y][x] != 0)
							program.display.drawPixel(px+x, py+y, BLACK);
						else
							program.display.drawPixel(px+x, py+y, WHITE);
			}
		}

		void drawCursor(int r, int c, int color) {
			int px = xsz * c;
			int py = ysz * r;
			program.display.drawLine(px, py, px+xsz-2, py, color);
			program.display.drawLine(px, py+ysz-2, px+xsz-2, py+ysz-2, color);
			program.display.drawLine(px, py, px, py+ysz-2, color);
			program.display.drawLine(px+xsz-2, py, px+xsz-2, py+ysz-2, color);
		}

		void drawGrid() {
			for (int n=0; n<gr-1; n++) {
				int px = ysz*n+ysz-1;
				int py = xsz*n+xsz-1;
				program.display.drawLine(0, px, xl, px, BLACK);
				program.display.drawLine(py, 0, py, yl, BLACK);
			}
		}

		void redrawBoard() {
			drawGrid();
			for (int r=0; r<gr; r++)
				for (int c=0; c<gr; c++)
					drawChip(r, c, program.board[c][r]);
			drawCursor(program.cursorY, program.cursorX, BLACK);
			program.display.display();
		}

		bool mkTurn() {
			GAME_TURN turn = {program.mycolor, program.cursorX, program.cursorY};
			if (validate_turn(program.board, &turn) == E_OK) {
				make_turn(program.board, &turn);
				program.mycolor = ALTER_COLOR(program.mycolor);
				return true;
			} else {
				return false;
			}
		}

	public:
		virtual void draw() override {
			program.display.clearDisplay();
			redrawBoard();
		};
		virtual Event handleEvent(Event event) override {
			Event rc = Event::EV_NONE;
			if (game_is_over(program.board))
				return rc;

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
				if (!mkTurn())
					return rc;
				break;
			default:
				return rc;
			}
			if ((dx != 0 || dy != 0) &&
				program.cursorX+dx >= 0 && program.cursorX+dx < gr &&
				program.cursorY+dy >= 0 && program.cursorY+dy < gr)
			{
				drawCursor(program.cursorY, program.cursorX, WHITE);
				program.cursorX += dx;
				program.cursorY += dy;
			}
			redrawBoard();
			return rc;
		}
	};

	class ThinkWindow: public MyWindow {
	public:
		virtual Event handleEvent(Event event) override {
			return Event::EV_NONE;
		}
		virtual void draw() override {
		};
	};

	class AgainWindow: public MyWindow {
	public:
		virtual Event handleEvent(Event event) override {
			return Event::EV_NONE;
		}
		virtual void draw() override {
		};
	};

	StartWindow startWindow;
	GameWindow gameWindow;
	ThinkWindow thinkWindow;
	AgainWindow againWindow;

	MyWindow *mainWindow;

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
	
	CHIP_COLOR mycolor;
	
	GAME_STATE board __attribute__ ((aligned (8)));
	
public:
	MyProgram():Program(),
				startWindow(),
				gameWindow(),
				thinkWindow(),
				againWindow(),
				cursorX(3),
				cursorY(3),
				mycolor(COLOR_WHITE)
	{
		for (int i=0; i<gr; i++)
			for (int j=0; j<gr; j++)
				board[i][j] = COLOR_VACANT;
		board[gr/2-1][gr/2-1] = COLOR_WHITE;
		board[gr/2][gr/2] = COLOR_WHITE;
		board[gr/2-1][gr/2] = COLOR_BLACK;
		board[gr/2][gr/2-1] = COLOR_BLACK;
		mainWindow = &startWindow;
	}

	virtual void init() override {
		Program::init();
		display.clearDisplay();
		mainWindow->draw();
	}

	void setMainWindow(MyWindow *w) {
		mainWindow = w;
		mainWindow->draw();
	}

	/* we just have to redefine event handler */
	virtual Event handleEvent(Event event) override {
		return mainWindow->handleEvent(event);
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

MyProgram &MyProgram::MyWindow::program = mp;
