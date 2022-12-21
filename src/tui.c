#include "tui.h"
#include <curses.h>
#include <locale.h>
#include <ncurses.h>
#include <stdlib.h>

void end_tui(TuiCtx *t) {

  delwin(t->game_board);
	free(t);
  //	delwin(t->help_board);
  // delwin(t->score_board);
  endwin();
}

TuiCtx *create_tui_ui(int h, int w) {
  TuiCtx *t = malloc(sizeof(TuiCtx));

  int center_y = (LINES - h) / 2;
  int center_x = (COLS - w) / 2;

  // * 2 because of space between each and + 2 on each because of the borders
  t->game_board = newwin(h + 2, w * 2 + 3, center_y, center_x);
  t->cursor = (Point){.x = 0, .y = 0};

  wborder(t->game_board, '|', '|', '-', '-', '+', '+', '+', '+');

  return t;
}


void setup_tui(void) {
  initscr();
  cbreak();
  noecho();
  curs_set(0);          // hide cursor
  keypad(stdscr, true); // allow arrow keys
	setlocale(LC_ALL, "");
	start_color();

	init_pair(FLAG_PAIR, COLOR_WHITE, COLOR_RED);
	init_pair(EMPTY_PAIR, COLOR_WHITE, COLOR_CYAN);
	init_pair(REVEALED_PAIR, COLOR_BLACK, COLOR_YELLOW);
	init_pair(UNREVEALED_PAIR, COLOR_WHITE, COLOR_MAGENTA);
}
