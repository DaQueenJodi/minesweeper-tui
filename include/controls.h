#pragma once
#include "grid.h"
#include "tui.h"
#include "util.h"
#include <stdbool.h>

bool controls_down(char c);
bool controls_up(char c);
bool controls_left(char c);
bool controls_right(char c);
bool controls_dig(char c);
bool controls_flag(char c);
bool controls_quit(char c);
#define END_STATUS_NADA 0
#define END_STATUS_QUIT 1
#define END_STATUS_LOSS 2
// returns one of the above macros
int handle_controls(TuiCtx *tc, Tiles *ts, GlobalStats *gs);
