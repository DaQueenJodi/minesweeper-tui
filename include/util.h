#pragma once

#include <stddef.h>
#include <stdbool.h>


typedef struct {
	size_t x;
	size_t y;
} Point;


bool cmp_points(Point a, Point b);

int rand_range(int a, int b);
