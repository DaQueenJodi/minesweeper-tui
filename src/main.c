#include "controls.h"
#include "grid.h"
#include "tui.h"
#include "util.h"
#include <bits/pthreadtypes.h>
#include <curses.h>
#include <ncurses.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

pthread_mutex_t refresh_lock;
pthread_mutex_t update_quit_lock;
pthread_mutex_t update_win_lock;
#define REFRESH_WINDOW(win)                                                    \
  pthread_mutex_lock(&refresh_lock);                                           \
  wrefresh(win);                                                               \
  pthread_mutex_unlock(&refresh_lock);
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

    pthread_mutex_lock(&update_win_lock);
    update_timer(tc, time);
    pthread_mutex_unlock(&update_win_lock);
    REFRESH_WINDOW(tc->time);

    usleep(100000); // 1 milisecond

    pthread_mutex_lock(&update_quit_lock);
  }
  return NULL;
}

/* test randomness
int main(void) {
        srand(time(NULL));
  int nums[100];
  int ocurrances_arr[15] = {0};

  // this is bad but its for testing so shush
  for (int i = 0; i < 50; i++) {
    for (int i = 0; i < 100; i++) {
      nums[i] = rand_range(15);
    }
    for (int i = 0; i < 15; i++) {
      int ocurrances = 0;

      for (int j = 0; j < 100; j++) {
        if (nums[j] == i) {
          ocurrances += 1;
        }
      }
      ocurrances_arr[i] += ocurrances;
    }
  }
  for (int i = 0; i < 15; i++) {
    printf("%d occured %d times \n", i, ocurrances_arr[i]);
  }
        } */

typedef enum { GameWon, GameLoss, GameQuit } GameResult;

int main(int argc, char **argv) {

  if (argc < 4) {
    printf("usage: %s border_h border_w num_bombs [seed] \n", argv[0]);
		exit(1);
  }
  int board_h = atoi(argv[1]);
  int board_w = atoi(argv[2]);
  int num_bombs = atoi(argv[3]);
	
  unsigned int seed = time(NULL);
	
  if (argc > 5) {
    seed = atoi(argv[3]);
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

  TuiCtx *tc = create_tui_ui(board_h, board_w);

  if (pthread_mutex_init(&refresh_lock, NULL) != 0) {
    fprintf(stderr, "failed to create refresh mutex \n");
    exit(1);
  }

  if (pthread_mutex_init(&update_quit_lock, NULL) != 0) {
    fprintf(stderr, "failed to create update mutex \n");
    exit(1);
  }
  if (pthread_mutex_init(&update_win_lock, NULL) != 0) {
    fprintf(stderr, "failed to create update win lock mutex \n");
    exit(1);
  }
  pthread_t thread_id;
  int error = pthread_create(&thread_id, NULL, update_timer_thread, tc);
  if (error != 0) {
    fprintf(stderr, "failed to create thread: %s", strerror(error));
    exit(1);
  }

  Tiles *ts = make_grid(board_h, board_w, num_bombs, gs);

  bool quit = false;

  GameResult game_result;

  while (!quit) {
    pthread_mutex_lock(&update_win_lock);
    wclear(tc->game_board);
    print_tiles(ts, tc);
    pthread_mutex_unlock(&update_win_lock);
    REFRESH_WINDOW(tc->game_board);

    pthread_mutex_lock(&update_win_lock);
    wclear(tc->stats);
    update_stats(tc, gs);
    pthread_mutex_unlock(&update_win_lock);
    REFRESH_WINDOW(tc->stats);

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
    //	refresh();
    print_tiles(ts, tc);
    wrefresh(tc->game_board);
    // TODO: make this not hardcoded
    WINDOW *result_win = newwin(1, 20, 15, 50);
    if (game_result == GameLoss)
      wprintw(result_win, "you lost :pensive:");
    else
      wprintw(result_win, "you won!");
    wrefresh(result_win);
    while (true) {
      char c = getch();
      if (controls_quit(c)) {
        break;
      }
    }
    delwin(result_win);
  }

  free_tiles(ts);


	free(gs);
	
  end_tui(tc);

  return 0;
}
