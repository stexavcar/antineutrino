#ifndef _WHITEOUT_CAIRO_BACKEND
#define _WHITEOUT_CAIRO_BACKEND

#include <cstdlib>
#include <cairo.h>
#include "wtk.h"

namespace whiteout {

class PaintContext {
public:
  PaintContext(cairo_t *cr, int width, int height)
    : cr_(cr), width_(width), height_(height)  { }
  cairo_t *cr() { return cr_; }
  int width() { return width_; }
  int height() { return height_; }
private:
  cairo_t *cr_;
  int width_;
  int height_;
};

class CairoBackend : public wtk::ElementVisitor {
public:
  class Info {
  public:
    inline Info(CairoBackend* backend, int top, int left, int width,
        int height);
    inline ~Info();
    int top() { return top_; }
    int left() { return left_; }
    int width() { return width_; }
    int height() { return height_; }
  private:
    CairoBackend* backend_;
    int top_, left_, width_, height_;
    Info* prev_;
  };
  explicit CairoBackend(wtk::Graphics &graphics)
    : graphics_(graphics), current_(NULL), info_(NULL) { }
  void paint(PaintContext &context);
  virtual void visit_circle(wtk::Circle &that);
  virtual void visit_rect(wtk::Rect &that);
  wtk::Graphics &graphics() { return graphics_; }
private:
  wtk::Graphics &graphics_;
  PaintContext &current() { return *current_; }
  PaintContext *current_;
  Info &info() { return *info_; }
  Info *info_;
};

CairoBackend::Info::Info(CairoBackend* backend, int top, int left, int width,
    int height)
    : backend_(backend), top_(top), left_(left), width_(width), height_(height) {
  prev_ = backend->info_;
  backend->info_ = this;
}

CairoBackend::Info::~Info() {
  backend_->info_ = prev_;
}

} // namespace whiteout

#endif // _WHITEOUT_CAIRO_BACKEND
