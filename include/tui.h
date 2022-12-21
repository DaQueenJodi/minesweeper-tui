#pragma once
#include "util.h"
#include <ncurses.h>

typedef struct {
  WINDOW *game_board;
  Point cursor;
  // WINDOW *score;
  // WINDOW *help;
} TuiCtx;

#define FLAG_PAIR 1
#define EMPTY_PAIR 2
#define REVEALED_PAIR 3
#define UNREVEALED_PAIR 4

void setup_tui(void);
void end_tui(TuiCtx *t);
TuiCtx *create_tui_ui(int h, int w);


