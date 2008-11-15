#ifndef _WHITEOUT_CAIRO_BACKEND
#define _WHITEOUT_CAIRO_BACKEND

#include <cairo.h>

namespace whiteout {

class CairoBackend {
public:
  void paint(cairo_surface_t* surface);
};

} // namespace whiteout

#endif // _WHITEOUT_CAIRO_BACKEND
