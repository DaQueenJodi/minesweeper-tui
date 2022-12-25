#include "controls.h"
#include "grid.h"
#include "tui.h"
#include "util.h"
#include <bits/pthreadtypes.h>
#include <curses.h>
#include <ncurses.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define JODCCL_IMPLEMENTATION
#include "jodccl.h"

pthread_mutex_t refresh_lock;
pthread_mutex_t update_quit_lock;
struct timeval initial_time;

bool quit_timer = false;

static void *update_timer_thread(void *vargp) {
  struct timeval current_time;
  pthread_mutex_lock(&update_quit_lock);

  while (!quit_timer) {
    pthread_mutex_unlock(&update_quit_lock);
    TuiCtx *tc = (TuiCtx *)vargp;
    gettimeofday(&current_time, NULL);
    double time =
        (double)(current_time.tv_usec - initial_time.tv_usec) / 1000000 +
        (double)(current_time.tv_sec - initial_time.tv_sec);

    pthread_mutex_lock(&refresh_lock);
    werase(tc->time);
    update_timer(tc, time);
    wrefresh(tc->time);
    pthread_mutex_unlock(&refresh_lock);

    napms(100); // tenth of a second

    pthread_mutex_lock(&update_quit_lock);
  }
  return NULL;
}

typedef enum { GameWon, GameLoss, GameQuit } GameResult;

int main(int argc, char **argv) {

  (void)argc;

  bool extra_ui = false;
  int board_w;
  int board_h;
  int num_bombs;
  int seed = time(NULL);

  jcl_add_bool(&extra_ui, "extraui", "ui", "disables the extra ui elements",
               false);
  jcl_add_int(&board_h, "height", "h", "sets the height of the board",
              true);
  jcl_add_int(&board_w, "width", "w", "sets the width of the board", true);
  jcl_add_int(&seed, "seed", "s", "sets the seed used for board generation",
              false);
  jcl_add_int(&num_bombs, "bombs", "b", "sets the number of bombs on the board",
              true);
  JCLError err = jcl_parse_args(argv);

	if (err.err != JCL_PARSE_OK) {
		jcl_print_error(err);
		exit(1);
	}

  gettimeofday(&initial_time, NULL);

  srand(seed);

  FILE *seed_file = fopen("seeds.txt", "a");
  fprintf(seed_file, "%u\n", seed);

  setup_tui();

  GlobalStats *gs = malloc(sizeof(GlobalStats));

  gs->elapsed_time = 0;
  gs->flags_left = num_bombs;
  gs->tiles_left = (board_h * board_w) - num_bombs;
  gs->num_turns = 0;

  TuiCtx *tc = create_tui_ui(board_h, board_w, extra_ui);

  if (pthread_mutex_init(&refresh_lock, NULL) != 0) {
    fprintf(stderr, "failed to create refresh mutex \n");
    exit(1);
  }

  if (pthread_mutex_init(&update_quit_lock, NULL) != 0) {
    fprintf(stderr, "failed to create update mutex \n");
    exit(1);
  }

  pthread_t thread_id;
  if (extra_ui) {
    int error = pthread_create(&thread_id, NULL, update_timer_thread, tc);
    if (error != 0) {
      fprintf(stderr, "failed to create thread: %s", strerror(error));
      exit(1);
    }
  }

  Tiles *ts = make_grid(board_h, board_w, num_bombs, gs);

  bool quit = false;

  GameResult game_result;

  while (!quit) {
    pthread_mutex_lock(&refresh_lock);
    // we dont need to erase the board window since its always completely
    // overwritten in print_tiles
    print_tiles(ts, tc);
    wrefresh(tc->game_board);
    pthread_mutex_unlock(&refresh_lock);
    if (extra_ui) {
      pthread_mutex_lock(&refresh_lock);
      werase(tc->stats);
      update_stats(tc, gs);
      wrefresh(tc->stats);
      pthread_mutex_unlock(&refresh_lock);
    }
    int end_status = handle_controls(tc, ts, gs);
    switch (end_status) {
    case END_STATUS_LOSS: {
      quit = true;
      game_result = GameLoss;
      break;
    }
    case END_STATUS_QUIT: {
      quit = true;
      game_result = GameQuit;
      break;
    }
    }

    if (gs->tiles_left == 0) {
      quit = true;
      game_result = GameWon;
      break;
    }
  }

  pthread_mutex_lock(&update_quit_lock);
  quit_timer = true;
  pthread_mutex_unlock(&update_quit_lock);

  if (game_result != GameQuit) {
    // erase these just because theyre not important
    werase(tc->stats);
    werase(tc->help);
    wrefresh(tc->stats);
    wrefresh(tc->help);
    print_tiles(ts, tc);
    wrefresh(tc->game_board);
    if (game_result == GameLoss)
      wprintw(tc->game_result, "you lost :pensive:");
    else
      wprintw(tc->game_result, "you won!");
    wrefresh(tc->game_result);
    while (true) {
      char c = getch();
      if (controls_quit(c)) {
        break;
      }
    }
  }

  free_tiles(ts);

  free(gs);

  end_tui(tc);

  return 0;
}
