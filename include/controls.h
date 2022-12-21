#pragma once
#include "grid.h"
#include "tui.h"
#include <stdbool.h>

bool controls_down(char c);
bool controls_up(char c);
bool controls_left(char c);
bool controls_right(char c);
bool controls_dig(char c);
bool controls_flag(char c);

#define END_STATUS_WIN  0
#define END_STATUS_LOSS 1
#define END_STATUS_NADA 2
#define END_STATUS_QUIT 3
// returns one of the above macros
int handle_controls(TuiCtx *tc, Tiles *ts);
