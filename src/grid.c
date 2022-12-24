#include "grid.h"
#include "tui.h"
#include "util.h"
#include <assert.h>
#include <curses.h>
#include <ncurses.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static bool point_already_there(Point *ps, size_t len, Point p) {
  for (size_t i = 0; i < len; i++) {
    Point temp = ps[i];
    if (temp.x == p.x && temp.y == p.y) {
      return true;
    }
  }
  return false;
}

static Point *pick_bomb_slots(uint8_t h, uint8_t w, uint8_t n) {
  Point *points = malloc(sizeof(Point) * n);
  for (uint8_t i = 0; i < n; i++) {
    Point p = (Point){.x = rand_range(w - 1), .y = rand_range(h - 1)};
    while (point_already_there(points, n, p)) {
      p = (Point){.x = rand_range(w - 1), .y = rand_range(h - 1)};
    }
    points[i] = p;
  }

  return points;
}

static size_t get_adj_tiles(Tiles *ts, Point p, Point *ps) {
  const size_t width = ts->width - 1;
  const size_t height = ts->height - 1;

  size_t length = 0;
#define ADD_POINT(add0, add1)                                                  \
  ps[length++] = (Point){.x = p.x + add0, .y = p.y + add1};

  if (p.x > 0)
    ADD_POINT(-1, 0);
  if (p.x < width)
    ADD_POINT(1, 0);
  if (p.y > 0)
    ADD_POINT(0, -1);
  if (p.y < height)
    ADD_POINT(0, 1);
  if (p.x > 0 && p.y > 0)
    ADD_POINT(-1, -1);
  if (p.x > 0 && p.y < height)
    ADD_POINT(-1, 1);
  if (p.x < width && p.y > 0)
    ADD_POINT(1, -1);
  if (p.x < width && p.y < height)
    ADD_POINT(1, 1);

  return length;
}

static size_t get_num_adj_mines(Tiles *ts, Point p) {

  assert(p.x <= ts->width - 1);
  assert(p.x >= 0);
  assert(p.y <= ts->height - 1);
  assert(p.y >= 0);

  Point ps[8];
  size_t length = get_adj_tiles(ts, p, ps);

  size_t counter = 0;
  for (size_t i = 0; i < length; i++) {
    Point adj = ps[i];
    if (tiles_get_pos(ts, adj)->is_mine)
      counter += 1;
  }

  return counter;
}

Tiles *make_grid(size_t height, size_t width, uint8_t num_bombs,
                 GlobalStats *gs) {
  Tiles *tiles = new_tiles(height, width);
  Point *bombs = pick_bomb_slots(height, width, num_bombs);
  for (uint8_t i = 0; i < num_bombs; i++) {
    tiles_get_pos(tiles, bombs[i])->is_mine = true;
  }
  free(bombs);

  for (size_t h = 0; h < height; h++) {
    for (size_t w = 0; w < width; w++) {
      Point p = (Point){.x = w, .y = h};
      tiles_get_pos(tiles, p)->num = get_num_adj_mines(tiles, p);
    }
  }

  gs->flags_left = num_bombs;

  return tiles;
}

Tile *tiles_get_pos(Tiles *ts, Point p) {
  return ts->tiles + p.x + (p.y * ts->width);
}

Tiles *new_tiles(size_t h, size_t w) {
  Tiles *ts = malloc(sizeof(Tiles));

  ts->width = w;
  ts->height = h;
  ts->tiles = malloc(sizeof(Tile) * h * w);
  for (size_t i = 0; i < w * h; i++) {
    ts->tiles[i] = (Tile){
        .is_flagged = false, .is_mine = false, .num = 0, .revealed = false};
  }

  return ts;
}
int CURRENT_COLOR_PAIR;
#define COLOR_PAIR_ON(n)                                                       \
  wattron(win, COLOR_PAIR(n));                                                 \
  CURRENT_COLOR_PAIR = n;
#define COLOR_PAIR_OFF() wattroff(win, COLOR_PAIR(CURRENT_COLOR_PAIR));

void print_tiles(Tiles *ts, TuiCtx *tc) {
  WINDOW *win = tc->game_board;
  for (size_t h = 0; h < ts->height; h++) {
    wmove(win, h, 0);
    wprintw(win, " ");
    for (size_t w = 0; w < ts->width; w++) {
      Point p = (Point){.x = w, .y = h};
      if (cmp_points(p, tc->cursor)) {
        wattron(win, A_STANDOUT);
      }
      Tile *t = tiles_get_pos(ts, p);
      if (t->is_flagged) {
        COLOR_PAIR_ON(FLAG_PAIR);
        wprintw(win, ">");
      } else if (t->revealed) {
        if (t->is_mine) {
          wprintw(win, "*");
        } else {
          if (t->num > 0) {
            COLOR_PAIR_ON(REVEALED_PAIR);
            wprintw(win, "%d", t->num);
          } else {
            COLOR_PAIR_ON(EMPTY_PAIR);
            wprintw(win, " ");
          }
        }
      } else {
        COLOR_PAIR_ON(UNREVEALED_PAIR);
        wprintw(win, " ");
      }

      wprintw(win, " ");
      wattroff(win, A_STANDOUT);
      COLOR_PAIR_OFF();
    }
  }
  wprintw(win, " ");
}

void free_tiles(Tiles *ts) {
  free(ts->tiles);
  free(ts);
}

static void next_point(Tiles *ts, Point *p) {
  if (p->x == ts->width - 1) {
    p->x = 0;
    p->y += 1;
  } else {
    p->x += 1;
  }
}
// if first click is a mine, get the point to place that mine to
static Point displace_mine_point(Tiles *ts) {
  Tile *t;
  // start in top left just like real minesweeper
  Point curr_p = (Point){.x = 0, .y = 0};
  // TODO: make this work like real minesweeper instead of going in order
  // even though it doesnt really matter
  while ((t = tiles_get_pos(ts, curr_p))) {
    if (!t->is_mine) {
      return curr_p;
    }
    next_point(ts, &curr_p);
  }
  assert(0 && "unreachable");
}

// TODO: make this more efficient and not recursive
bool tiles_dig(Tiles *ts, Point p, GlobalStats *gs) {
  Tile *t = tiles_get_pos(ts, p);
  if (gs->num_turns == 1) {
    if (t->is_mine) {
      Point mine_p = displace_mine_point(ts);
      Tile *mine_tile = tiles_get_pos(ts, mine_p);
      mine_tile->is_mine = true;
      t->is_mine = false;
      Point ps[8];
      size_t length = get_adj_tiles(ts, mine_p, ps);
      for (size_t i = 0; i < length; i++) {
        tiles_get_pos(ts, ps[i])->num += 1;
      }
			length = get_adj_tiles(ts, p, ps);
      for (size_t i = 0; i < length; i++) {
        tiles_get_pos(ts, ps[i])->num -= 1;
      }
			t->revealed = true;
    }
  } else if (t->is_mine) {
    t->revealed = true;
    return false;
  }
	
  if (!t->revealed) {
    t->revealed = true;
    gs->tiles_left -= 1;
  }

  if (t->num > 0) { // dont continue if not a 0
    return true;
  }

  Point ps[8];
  size_t length = get_adj_tiles(ts, p, ps);

  for (size_t i = 0; i < length; i++) {
    Tile *t = tiles_get_pos(ts, ps[i]);
    if (!t->revealed) {
      if (t->num == 0) {
        tiles_dig(ts, ps[i], gs); // only recurse if its a 0
      } else {
        t->revealed = true;
        gs->tiles_left -= 1;
      }
    }
  }
  return true;
}

void complete_tile_flags(Tiles *ts, Point p, GlobalStats *gs) {
  Tile *t = tiles_get_pos(ts, p);
  Point ps[8];
  size_t length = get_adj_tiles(ts, p, ps);
  size_t to_replace_len = 0;
  Point to_replace_ps[8];

  size_t flagged_counter = 0;
  for (size_t i = 0; i < length; i++) {
    Tile *t = tiles_get_pos(ts, ps[i]);
    if (t->is_flagged)
      flagged_counter += 1;
    else if (!t->revealed) // make sure we dont replace revealed tiles
      to_replace_ps[to_replace_len++] = ps[i];
  }
  // if flags are there, no need to continue
  if (flagged_counter >= t->num) {
    return;
  }

  // if the number of missing flags isnt the same as the number of empty slots,
  // do nothing
  if (to_replace_len > t->num - flagged_counter) {
    return;
  }

  // flag all the tiles
  for (size_t i = 0; i < to_replace_len; i++) {
    gs->flags_left -= 1;
    tiles_get_pos(ts, to_replace_ps[i])->is_flagged = true;
  }
}

bool complete_flagged_tile(Tiles *ts, Point p, GlobalStats *gs) {
  // check if the right number of tiles are flagged
  // if the right number of tiles are flagged, dig the rest of the mines

  Tile *t = tiles_get_pos(ts, p);
  Point ps[8];

  size_t flag_counter = 0;

  size_t unflagged_p_len = 0;
  Point unflagged_ps[8];

  size_t length = get_adj_tiles(ts, p, ps);
  for (size_t i = 0; i < length; i++) {
    if (tiles_get_pos(ts, ps[i])->is_flagged)
      flag_counter += 1;
    else
      unflagged_ps[unflagged_p_len++] = ps[i];
  }

  // if there is the right number of flags
  if (flag_counter >= t->num) {
    for (size_t i = 0; i < unflagged_p_len; i++) {
      if (!tiles_dig(ts, unflagged_ps[i], gs))
        return false;
    }
  }
  return true;
}
