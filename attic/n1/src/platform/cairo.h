#ifndef _PLATFORM_CAIRO
#define _PLATFORM_CAIRO

#include "plankton/plankton.h"
#include <cairo.h>

namespace neutrino {

class PaintContext {
public:
  PaintContext(word width, word height, cairo_t *cr)
    : width_(width)
    , height_(height)
    , cr_(cr) { }
  word width() { return width_; }
  word height() { return height_; }
  cairo_t *cairo() { return cr_; }
private:
  word width_;
  word height_;
  cairo_t *cr_;
};

class Cairo {
public:
  void paint(PaintContext &context);
};

} // namespace neutrino

#endif // _PLATFORM_CAIRO
