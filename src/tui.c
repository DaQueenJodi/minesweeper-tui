#include "tui.h"
#include "util.h"
#include <curses.h>
#include <locale.h>
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>

void end_tui(TuiCtx *t) {

  delwin(t->game_board);
	delwin(t->help);
	delwin(t->stats);
  free(t);
  endwin();
}

static void print_help(WINDOW *help);

TuiCtx *create_tui_ui(int h, int w) {
  TuiCtx *t = malloc(sizeof(TuiCtx));

  int window_h = h;
  int window_w = w * 3; // " n "

  int center_y = (LINES - window_h) / 2;
  int center_x = (COLS - window_w) / 2;


  t->game_board = newwin(window_h, window_w, center_y, center_x);
  t->cursor = (Point){.x = 0, .y = 0};
	t->stats = newwin(4, 30, center_y + 10, center_x - 40);
	t->time = newwin(1, 40, center_y - 2, center_x + (window_w / 2) - 15);
  t->help = newwin(50, 100, center_y, center_x - 40);
	// this is static so we might as well just do it here
	print_help(t->help);
	wrefresh(t->help);
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
  refresh(); // this is apparently important since otherwise getch() stop the
             // windows from displaying at first

  init_pair(FLAG_PAIR, COLOR_WHITE, COLOR_RED);
  init_pair(EMPTY_PAIR, COLOR_WHITE, COLOR_CYAN);
  init_pair(REVEALED_PAIR, COLOR_BLACK, COLOR_YELLOW);
  init_pair(UNREVEALED_PAIR, COLOR_WHITE, COLOR_MAGENTA);
}

void update_stats(TuiCtx *tc, GlobalStats *gs) {

  WINDOW *win = tc->stats;
  // 0, 0 is handled in update_timer_stats, below
  wmove(win, 1, 0);
  wprintw(win, "turns: %zu", gs->num_turns);
  wmove(win, 2, 0);

	// TODO: add leading 0s to fix issue with it getting
  // to lower digits, will need a proper fix eventually
  wprintw(win, "flags left: %02d", gs->flags_left);
	wmove(win, 3, 0);
	wprintw(win, "tiles left: %03zu", gs->tiles_left);
}

void update_timer(TuiCtx *tc, double time) {
  size_t minutes = time / 60;
  double seconds = time - (double)(minutes * 60);
  WINDOW *win = tc->time;
  wmove(win, 0, 0);
  wprintw(win, "time: %zu:%.1f", minutes, seconds);
}


static void print_help(WINDOW *help) {
	wmove(help, 0, 0);
	wprintw(help, "h/j/k/l: move left/down/up/right");
	wmove(help, 1, 0);
	wprintw(help, "q: quit");
	wmove(help, 2, 0);
	wprintw(help, "a/<Space>: flag");
	wmove(help, 3, 0);
	wprintw(help, "s/<RET>: dig");
}
