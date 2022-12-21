#include "controls.h"
#include "grid.h"
#include "tui.h"
#include <curses.h>

#define BOARD_W 16
#define BOARD_H 16

int main(void) {

  setup_tui();

  TuiCtx *tc = create_tui_ui(BOARD_H, BOARD_W);

  Tiles *ts = make_grid(BOARD_H, BOARD_W, 40);

  bool quit = false;

  while (!quit) {
    print_tiles(ts, tc);
    wrefresh(tc->game_board);
    int end_status = handle_controls(tc, ts);
    switch (end_status) {
    case END_STATUS_LOSS: {
      quit = true;
      puts("welp you lost :pensive:");
      break;
    }
    case END_STATUS_QUIT: {
      quit = true;
      break;
    }
    case END_STATUS_WIN: {
      quit = true;
      puts("nice, you won");
      break;
    }
    }
  }

  free_tiles(ts);

  end_tui(tc);
	
  return 0;
}
