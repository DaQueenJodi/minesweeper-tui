#pragma once

#include "tui.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "util.h"

typedef struct {
  bool is_flagged;
  bool is_mine;
  uint8_t num;
	bool revealed;
} Tile;


typedef struct {
  Tile *tiles;
  size_t height;
  size_t width;
} Tiles;

Tiles *new_tiles(size_t h, size_t w);

Tiles *make_grid(size_t h, size_t w, uint8_t num_bombs, GlobalStats *gs);
Tile *tiles_get_pos(Tiles *ts, Point p);

void print_tiles(Tiles *ts, TuiCtx *tc);

void free_tiles(Tiles *ts);

bool tiles_dig(Tiles *ts, Point p, GlobalStats *gs);

bool complete_flagged_tile(Tiles *ts, Point p, GlobalStats *gs);
void complete_tile_flags(Tiles *ts, Point p, GlobalStats *gs);
