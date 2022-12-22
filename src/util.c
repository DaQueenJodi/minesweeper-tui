#include "util.h"
#include <assert.h>
#include <stdlib.h>

bool cmp_points(Point a, Point b) {
	return a.x == b.x && a.y == b.y;
}

// TODO: make this have better randomness
int rand_range(int e) {
	return rand() % (e + 1);
}
