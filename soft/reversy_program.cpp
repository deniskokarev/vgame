/**
 * Revery game for our STM32 mini-console
 * @author Denis Kokarev
 */

#include "program.h"

/*
 * enable if we want to start with the Autotest window
 * useful for benchmarking and debugging
 */ 
//#define AUTOTEST

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

/**
 * A game of reversy for our mini-console
 * contains three Windows - Start, Game and Again
 * It also contains the game board
 */
class MyProgram: public WProgram {
protected:
	/**
	 * All MyWindows to have a reference to our MyProgram
	 * to access its fields, such as display and game
	 * board
	 */
	class MyWindow: public Window {
	protected:
		/**
		 * A direct hardcoded access to master program via
		 * the same static reference for all windows is just fine
		 */
		static MyProgram &program;
	};

protected:
	/**
	 * Our splash window, simply displays about message
	 * and offers to start the game
	 */
	class StartWindow: public MyWindow {
	protected:
		char *label; // cannot be const due to stupid Adafruit lib
		Adafruit_GFX_Button startButton; /*! GUI screen button */
	public:
		/**
		 * initialize a window and START button
		 */
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
		/**
		 * proceed to Game window on KEY_ENTER event
		 */
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
		/**
		 * Display our staring message and on-screen START button
		 */
		virtual void draw() override {
			program.display.print("Reversy v0.9");
			startButton.drawButton();
			program.display.display();
		};
	};

	/**
	 * our main Game window, which handles user keys and paints the board
	 */
	class GameWindow: public MyWindow {
	protected:
		/**
		 * Paint one chip
		 * @param r - row number
		 * @param c - column number
		 * @param color - what is the chip color
		 * @param[out] nWhite - will be incremented if the color is white
		 * @param[out] nBlack - will be incremented if the color is black
		 */
		void drawChip(int r, int c, CHIP_COLOR color, int &nWhite, int &nBlack) {
			int px = cell_xsz * c + 1;
			int py = cell_ysz * r + 1;
			if (color == COLOR_VACANT) {
				for (int y=0; y<cell_ysz-2; y++)
					for (int x=0; x<cell_xsz-2; x++)
						program.display.drawPixel(px+x, py+y, WHITE);
			} else {
				const unsigned char (&chip)[cell_ysz-2][cell_xsz-2] = (color == COLOR_POS)?whiteChip:blackChip;
				switch (color) {
				case COLOR_POS:
					nWhite++;
					break;
				case COLOR_NEG:
					nBlack++;
					break;
				}
				for (int y=0; y<cell_ysz-2; y++)
					for (int x=0; x<cell_xsz-2; x++)
						if (chip[y][x] != 0)
							program.display.drawPixel(px+x, py+y, BLACK);
						else
							program.display.drawPixel(px+x, py+y, WHITE);
			}
		}
		/**
		 * Paint the current cursor location with the given color
		 * @param r - row number
		 * @param c - column number
		 * @param color - paint with what color
		 */
		void drawCursor(int r, int c, int color) {
			int px = cell_xsz * c;
			int py = cell_ysz * r;
			program.display.drawLine(px, py, px+cell_xsz-2, py, color);
			program.display.drawLine(px, py+cell_ysz-2, px+cell_xsz-2, py+cell_ysz-2, color);
			program.display.drawLine(px, py, px, py+cell_ysz-2, color);
			program.display.drawLine(px+cell_xsz-2, py, px+cell_xsz-2, py+cell_ysz-2, color);
		}
		/**
		 * Paint empty 8x8 grid on the screen
		 */
		void drawGrid() {
			for (int n=0; n<board_dim-1; n++) {
				int px = cell_ysz*n+cell_ysz-1;
				int py = cell_xsz*n+cell_xsz-1;
				program.display.drawLine(0, px, board_xsz, px, BLACK);
				program.display.drawLine(py, 0, py, board_ysz, BLACK);
			}
		}
		/**
		 * Paint the current score on the right side of the screen
		 * @param nWhite - number of white chips
		 * @param nBlack - number of black chips
		 */
		void drawScore(int nWhite, int nBlack) {
			char sw[3];
			char sb[3];
			*(lltoan(sw, nWhite, 2)) = 0;
			*(lltoan(sb, nBlack, 2)) = 0;
			int16_t x, y;
			uint16_t w, h;
			program.display.getTextBounds(sw, 0, 0, &x, &y, &w, &h);
			x = (board_dim*cell_xsz)+(program.display.width()-(board_dim*cell_xsz)-w)/2;
			y = (program.display.height()-h*3)/2;
			program.display.setCursor(x, y);
			program.display.setTextColor(BLACK, WHITE);
			program.display.print(sw);
			program.display.setCursor(x, y+2*h);
			program.display.setTextColor(WHITE, BLACK);
			program.display.print(sb);
		}
		/**
		 * Paint the grid, all chips and score
		 * Push the image to the screen
		 */
		void redrawBoard() {
			drawGrid();
			int nWhite = 0;
			int nBlack = 0;
			for (int r=0; r<board_dim; r++)
				for (int c=0; c<board_dim; c++)
					drawChip(r, c, program.board.b[c][r], nWhite, nBlack);
			drawCursor(program.cursorY, program.cursorX, BLACK);
			drawScore(nWhite, nBlack);
			program.display.display();
		}
		/**
		 * Execute when user pressed ENTER.
		 * Make a turn by player at the current cursor position
		 * and immediately perform the turn by the computer
		 */
		bool mkTurn() {
			GAME_TURN turn = {program.mycolor, program.cursorX, program.cursorY};
			if (validate_turn(&program.board, &turn) == E_OK) {
				make_turn(&program.board, &turn);
				redrawBoard();
				GAME_TURN availableTurns[board_dim*board_dim];
				GAME_TURN machineTurn;
				int n;
				while ((n=make_turn_list(availableTurns, &program.board, ALTER_COLOR(program.mycolor)))>0) {
					find_best_turn(&machineTurn, &program.board, ALTER_COLOR(program.mycolor), program.level);
					make_turn(&program.board, &machineTurn);
					if ((n=make_turn_list(availableTurns, &program.board, program.mycolor))>0)
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
		/**
		 * Draw game board and push it to the screen
		 */
		virtual void draw() override {
			program.display.clearDisplay();
			redrawBoard();
		};
		/**
		 * Handle user keys by moving cursor on arrows and making a turn on enter
		 */
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
				program.cursorX+dx >= 0 && program.cursorX+dx < board_dim &&
				program.cursorY+dy >= 0 && program.cursorY+dy < board_dim)
			{
				drawCursor(program.cursorY, program.cursorX, WHITE);
				program.cursorX += dx;
				program.cursorY += dy;
			}
			redrawBoard();
			return rc;
		}
	};

#ifdef AUTOTEST
	/**
	 * Optional window for autotesting - when this window becomes main and there is
	 * a CUSTOM_1 event in the events queue, the computer starts playing with itself
	 * until game is over. Then regular Game window becomes active
	 */
	class TestGameWindow: public GameWindow {
		/**
		 * if the events queue has CUSTOM_1 event start playing with itself until the game is over
		 * then activate regular game window
		 */ 
		virtual Event handleEvent(Event event) override {
			if (event == Event::EV_CUSTOM+1) {
				GAME_TURN availableTurns[board_dim*board_dim];
				GAME_TURN machineTurn;
				int n;
				while ((n=make_turn_list(availableTurns, &program.board, program.mycolor))>0) {
					find_best_turn(&machineTurn, &program.board, program.mycolor, program.level);
					make_turn(&program.board, &machineTurn);
					if ((n=make_turn_list(availableTurns, &program.board, ALTER_COLOR(program.mycolor)))>0)
						break;
					redrawBoard();
				}
				redrawBoard();
				if (n<=0)
					program.gameIsOver = true;
				else
					events->put(event);
				program.mycolor = ALTER_COLOR(program.mycolor);
			}
			return GameWindow::handleEvent(event);
		}
	};
	TestGameWindow testGameWindow;
#endif

	/**
	 * AgainWindow displayed after the game is over
	 * and shows the score and offers another round
	 */
	class AgainWindow: public MyWindow {
	protected:
		const char *message;			/*! game result */
		char *label; // cannot be const due to stupid Adafruit lib
		Adafruit_GFX_Button againButton;	/*! GUI screen button */
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
		/**
		 * Activate Game window upon KEY_ENTER
		 */
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
		/**
		 * Select which message to display, WIN/LOSE/DRAW
		 */
		void updateMessage() {
			int cu = chips_count(&program.board, program.mycolor);
			int cnu = chips_count(&program.board, ALTER_COLOR(program.mycolor));
			if (cu > cnu)
				message = "You WIN!!!";
			else if (cu < cnu)
				message = "You LOSE!!";
			else
				message = "!!!DRAW!!!";
		}
		/**
		 * display the message and the GUI button
		 */
		virtual void draw() override {
			program.display.clearDisplay();
			program.display.setCursor(0, 0);
			program.display.setTextColor(BLACK, WHITE);
			program.display.print(message);
			againButton.drawButton();
			program.display.display();
		};
	};

	/**
	 * Splash window
	 */
	StartWindow startWindow;
	/**
	 * Main Game window
	 */
	GameWindow gameWindow;
	/**
	 * End of the game status window
	 */
	AgainWindow againWindow;

protected:
	/**
	 * board size == 8
	 */
	static constexpr int board_dim = MAX_DIM;
	/**
	 * height of the single cell on the screen
	 */
	static constexpr int cell_ysz = LCDHEIGHT/board_dim;
	/**
	 * height of entire board on the screen
	 */
	static constexpr int board_ysz = cell_ysz*board_dim-1;
	/**
	 * width of the single cell, which is a bit greater than height due to horizontal screen stretch
	 * to make it appear square
	 */
	static constexpr int cell_xsz = cell_ysz+1;
	/**
	 * width of entire board
	 */
	static constexpr int board_xsz = cell_xsz*board_dim-1;
	/**
	 * White chip sprite
	 */
	static const unsigned char whiteChip[cell_ysz-2][cell_xsz-2];
	/**
	 * Black chip sprite
	 */
	static const unsigned char blackChip[cell_ysz-2][cell_xsz-2];
	/**
	 * game board with all the chips on it
	 */
	GAME_STATE board;
	/**
	 * Game cursor position
	 */ 
	signed char cursorX, cursorY;
	/**
	 * player's color
	 */
	CHIP_COLOR mycolor;
	/**
	 * flag to be set when game is over
	 */
	bool gameIsOver;
	/**
	 * Computer level. Means how many steps deep the computer will be looking
	 * to find the best turn (default=5)
	 */
	char level;
public:
	/**
	 * perform minimal initialization
	 */
	MyProgram():WProgram(),
				cursorX(3),
				cursorY(3),
				mycolor(COLOR_POS)
	{
	}
	/**
	 * Reset game board and set focus to the StartWindow
	 */
	virtual void init() override {
		Program::init();
#ifdef AUTOTEST		
		setMainWindow(&testGameWindow);
		events->put(Event::EV_CUSTOM+1);
#else
		setMainWindow(&startWindow);
#endif
		startNewGame();
		display.clearDisplay();
		mainWindow->draw();
		level = 5;
	}
	/**
	 * Wipe out game board
	 */
	void startNewGame() {
		cursorX = 3;
		cursorY = 3;
		mycolor = COLOR_POS;
		gameIsOver = false;
		for (int i=0; i<board_dim; i++)
			for (int j=0; j<board_dim; j++)
				board.b[i][j] = COLOR_VACANT;
		board.b[board_dim/2-1][board_dim/2-1] = COLOR_POS;
		board.b[board_dim/2][board_dim/2] = COLOR_POS;
		board.b[board_dim/2-1][board_dim/2] = COLOR_NEG;
		board.b[board_dim/2][board_dim/2-1] = COLOR_NEG;
	}
	
};

/**
 * White chip sprite
 */
const unsigned char MyProgram::whiteChip[cell_ysz-2][cell_xsz-2] = {
	{0, 1, 1, 0},
	{1, 0, 0, 1},
	{0, 1, 1, 0},
};
/**
 * Black chip sprite
 */
const unsigned char MyProgram::blackChip[cell_ysz-2][cell_xsz-2] = {
	{0, 1, 1, 0},
	{1, 1, 1, 1},
	{0, 1, 1, 0},
};
/**
 * We must instantiate our program to register it into execution event loop
 */
MyProgram mp;
/**
 * All our windows simply refer to our program
 */
MyProgram &MyProgram::MyWindow::program = mp;
