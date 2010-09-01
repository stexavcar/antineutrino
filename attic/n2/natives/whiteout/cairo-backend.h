#ifndef _WHITEOUT_CAIRO_BACKEND
#define _WHITEOUT_CAIRO_BACKEND

#include <cstdlib>
#include <cairo.h>

namespace whiteout {

class CairoBackend {
public:
  CairoBackend();
  void initialize(int width, int height);
  void open();
private:
  int width_;
  int height_;
  class Data;
  Data *data() { return data_; }
  Data *data_;
};

} // namespace whiteout

#endif // _WHITEOUT_CAIRO_BACKEND
