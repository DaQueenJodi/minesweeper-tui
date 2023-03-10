#include "controls.h"
#include "grid.h"
#include "tui.h"
#include "util.h"
#include <sys/types.h>
#include <curses.h>
#include <ncurses.h>
#include <pthread.h>
#include <stddef.h>

bool controls_down(char c) {
  switch (c) {
  case 'j':
    return true; // TODO: add more
  }
  return false;
}

bool controls_up(char c) {
  switch (c) {
  case 'k':
    return true;
  }
  return false;
}

bool controls_left(char c) {
  switch (c) {
  case 'h':
    return true;
  }
  return false;
}

bool controls_right(char c) {
  switch (c) {
  case 'l':
    return true;
  }
  return false;
}

bool controls_quit(char c) {
  switch (c) {
  case 'q':
    return true;
  }
  return false;
}

bool controls_flag(char c) {
  switch (c) {
  case ' ':
    return true;
  case 'a':
    return true;
  }
  return false;
}

bool controls_dig(char c) {
  switch (c) {
  case '\n':
    return true;
  case 's':
    return true;
  }
  return false;
}

extern pthread_mutex_t refresh_lock;

int handle_controls(TuiCtx *tc, Tiles *ts, GlobalStats *gs) {
  char c;
  pthread_mutex_lock(&refresh_lock);
  c = getch();
  pthread_mutex_unlock(&refresh_lock);
	if (c == ERR) {
    return END_STATUS_NADA;
  }
  if (controls_left(c)) {
    if (tc->cursor.x > 0)
      tc->cursor.x -= 1;
  } else if (controls_down(c)) {
    if (tc->cursor.y < ts->height - 1)
      tc->cursor.y += 1;
  } else if (controls_up(c)) {
    if (tc->cursor.y > 0)
      tc->cursor.y -= 1;
  } else if (controls_right(c)) {
    if (tc->cursor.x < ts->width - 1)
      tc->cursor.x += 1;
  } else if (controls_quit(c)) {
    return END_STATUS_QUIT;
  } else if (controls_flag(c)) {
    Tile *t = tiles_get_pos(ts, tc->cursor);
    if (!t->revealed) {
      if (t->is_flagged) {
        t->is_flagged = false;
        gs->flags_left += 1; // add back the flag
      } else if (!t->is_flagged) {
        t->is_flagged = true;
        gs->flags_left -= 1; // remove a flag
      }
    } else
      complete_tile_flags(ts, tc->cursor, gs);
  } else if (controls_dig(c)) {
    Tile *t = tiles_get_pos(ts, tc->cursor);
    if (!t->is_flagged) {
      gs->num_turns += 1;
      if (!t->revealed) {
        if (!tiles_dig(ts, tc->cursor, gs))
          return END_STATUS_LOSS;
      } else if (!complete_flagged_tile(ts, tc->cursor, gs))
        return END_STATUS_LOSS;
    }
  }
  return END_STATUS_NADA;
}
