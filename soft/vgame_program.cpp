#include "program.h"

extern "C" {
#include "minimax.h"
}

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
			program.display.print("Reversy v0.9");
			startButton.drawButton();
			program.display.display();
		};
	};

	class GameWindow: public MyWindow {
	protected:
		void drawChip(int r, int c, CHIP_COLOR color, int &nWhite, int &nBlack) {
			int px = xsz * c + 1;
			int py = ysz * r + 1;
			if (color == COLOR_VACANT) {
				for (int y=0; y<ysz-2; y++)
					for (int x=0; x<xsz-2; x++)
						program.display.drawPixel(px+x, py+y, WHITE);
			} else {
				const unsigned char (&chip)[ysz-2][xsz-2] = (color == COLOR_WHITE)?whiteChip:blackChip;
				switch (color) {
				case COLOR_WHITE:
					nWhite++;
					break;
				case COLOR_BLACK:
					nBlack++;
					break;
				}
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

		void drawScore(int nWhite, int nBlack) {
			char sw[3];
			char sb[3];
			*(lltoan(sw, nWhite, 2)) = 0;
			*(lltoan(sb, nBlack, 2)) = 0;
			int16_t x, y;
			uint16_t w, h;
			program.display.getTextBounds(sw, 0, 0, &x, &y, &w, &h);
			x = (gr*xsz)+(program.display.width()-(gr*xsz)-w)/2;
			y = (program.display.height()-h*3)/2;
			program.display.setCursor(x, y);
			program.display.setTextColor(BLACK, WHITE);
			program.display.print(sw);
			program.display.setCursor(x, y+2*h);
			program.display.setTextColor(WHITE, BLACK);
			program.display.print(sb);
		}
		
		void redrawBoard() {
			drawGrid();
			int nWhite = 0;
			int nBlack = 0;
			for (int r=0; r<gr; r++)
				for (int c=0; c<gr; c++)
					drawChip(r, c, program.board[c][r], nWhite, nBlack);
			drawCursor(program.cursorY, program.cursorX, BLACK);
			drawScore(nWhite, nBlack);
			program.display.display();
		}

		bool mkTurn() {
			GAME_TURN turn = {program.mycolor, program.cursorX, program.cursorY};
			if (validate_turn(program.board, &turn) == E_OK) {
				make_turn(program.board, &turn);
				redrawBoard();
				GAME_TURN availableTurns[gr*gr];
				GAME_TURN machineTurn;
				int n;
				while ((n=make_turn_list(availableTurns, program.board, ALTER_COLOR(program.mycolor)))>0) {
					find_best_turn(&machineTurn, program.board, ALTER_COLOR(program.mycolor), 5);
					make_turn(program.board, &machineTurn);
					if ((n=make_turn_list(availableTurns, program.board, program.mycolor))>0)
						break;
					redrawBoard();
				}
				if (n<=0)
					program.gameIsOver = true;
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
			if (program.gameIsOver) {
				HAL_Delay(3000);
				program.againWindow.updateMessage();
				program.setMainWindow(&program.againWindow);
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

	class AgainWindow: public MyWindow {
	protected:
		const char *message;
		char *label; // cannot be const due to stupid Adafruit lib
		Adafruit_GFX_Button againButton;
	public:
		AgainWindow():MyWindow(),message((char*)""),label((char*)"Again") {
			int16_t x, y;
			uint16_t w, h;
			program.display.getTextBounds(label, 0, 0, &x, &y, &w, &h);
			againButton.initButton(&program.display,
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
				againButton.drawButton(true);
				program.display.display();
				HAL_Delay(300);
				program.startNewGame();
				program.setMainWindow(&program.gameWindow);
				break;
			default:
				break;
			}
			return Event::EV_NONE;
		}
		void updateMessage() {
			int cnt[COLOR_BLACK+1];
			for (int i=0; i<gr; i++)
				for (int j=0; j<gr; j++)
					cnt[program.board[i][j]]++;
			if (cnt[COLOR_WHITE] > cnt[COLOR_BLACK])
				message = "You WIN!!!";
			else if (cnt[COLOR_WHITE] < cnt[COLOR_BLACK])
				message = "You LOSE!!";
			else
				message = "!!!DRAW!!!";
		}
		virtual void draw() override {
			program.display.clearDisplay();
			program.display.setCursor(0, 0);
			program.display.setTextColor(BLACK, WHITE);
			program.display.print(message);
			againButton.drawButton();
			program.display.display();
		};
	};

	StartWindow startWindow;
	GameWindow gameWindow;
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

	bool gameIsOver;
public:
	MyProgram():Program(),
				startWindow(),
				gameWindow(),
				againWindow(),
				cursorX(3),
				cursorY(3),
				mycolor(COLOR_WHITE)
	{
		mainWindow = &startWindow;
	}

	virtual void init() override {
		Program::init();
		startNewGame();
		display.clearDisplay();
		mainWindow->draw();
	}

	void startNewGame() {
		cursorX = 3;
		cursorY = 3;
		mycolor = COLOR_WHITE;
		gameIsOver = false;
		for (int i=0; i<gr; i++)
			for (int j=0; j<gr; j++)
				board[i][j] = COLOR_VACANT;
		board[gr/2-1][gr/2-1] = COLOR_WHITE;
		board[gr/2][gr/2] = COLOR_WHITE;
		board[gr/2-1][gr/2] = COLOR_BLACK;
		board[gr/2][gr/2-1] = COLOR_BLACK;
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
