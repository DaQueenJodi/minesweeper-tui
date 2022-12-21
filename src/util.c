#include "util.h"
#include <assert.h>
#include <stdlib.h>

bool cmp_points(Point a, Point b) {
	return a.x == b.x && a.y == b.y;
}


int rand_range(int s, int e) {
  assert(s < e);
  return rand() % (e + 1 - s) + s;
}
