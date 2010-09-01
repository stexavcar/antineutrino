#include "utils/arena-inl.h"
#include "test-inl.h"

using namespace neutrino;

class Point {
public:
  Point(word x, word y) : x_(x), y_(y) { }
  word x_, y_;
};

TEST(arena) {
  Arena arena;
  buffer<Point*> points;
  for (word i = 0; i < 1024; i++) {
    Point *p = new (arena) Point(i, 1024 - i);
    assert p != ((void*) NULL);
    points.append(p);
  }
  for (word i = 0; i < 1024; i++) {
    Point *p = points[i];
    assert p->x_ == i;
    assert p->y_ == 1024 - i;
  }
}
