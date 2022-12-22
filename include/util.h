#pragma once

#include <stddef.h>
#include <stdbool.h>


typedef struct {
	size_t x;
	size_t y;
} Point;

typedef struct {
	int flags_left;
	size_t num_turns;
	double elapsed_time;
	size_t tiles_left;
} GlobalStats;

bool cmp_points(Point a, Point b);

int rand_range(int e);
