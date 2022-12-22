#pragma once
#include "util.h"
#include <curses.h>
#include <ncurses.h>
#include <stdint.h>
#include <time.h>

typedef struct {
  WINDOW *game_board;
  Point cursor;
  WINDOW *stats;
  WINDOW *help;
	WINDOW *time;
} TuiCtx;

#define FLAG_PAIR 1
#define EMPTY_PAIR 3
#define REVEALED_PAIR 3
#define UNREVEALED_PAIR 4

void setup_tui(void);
void end_tui(TuiCtx *t);
TuiCtx *create_tui_ui(int h, int w);
void update_stats(TuiCtx *tc, GlobalStats *gs);
void update_timer(TuiCtx *tc, double time);
