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
    inline Info(CairoBackend* backend, float left, float top, float width,
        float height);
    inline ~Info();
    float top() { return top_; }
    float left() { return left_; }
    float width() { return width_; }
    float height() { return height_; }
  private:
    CairoBackend* backend_;
    float left_, top_, width_, height_;
    Info* prev_;
  };
  explicit CairoBackend(wtk::Graphics &graphics)
    : graphics_(graphics), current_(NULL), info_(NULL) { }
  void paint(PaintContext &context);
  virtual void visit_circle(wtk::Circle &that);
  virtual void visit_rect(wtk::Rect &that);
  virtual void visit_container(wtk::Container &that);
  virtual void visit_text(wtk::Text &that);
  wtk::Graphics &graphics() { return graphics_; }
private:
  wtk::Graphics &graphics_;
  PaintContext &current() { return *current_; }
  PaintContext *current_;
  Info &info() { return *info_; }
  Info *info_;
};

CairoBackend::Info::Info(CairoBackend* backend, float left, float top,
    float width, float height)
    : backend_(backend), left_(left), top_(top), width_(width), height_(height) {
  prev_ = backend->info_;
  backend->info_ = this;
}

CairoBackend::Info::~Info() {
  backend_->info_ = prev_;
}

} // namespace whiteout

#endif // _WHITEOUT_CAIRO_BACKEND
