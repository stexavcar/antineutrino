#include "whiteout/cairo-backend.h"
#include <cairo.h>
#include <cstdio>

namespace whiteout {

#define kPi 3.141592645f

void CairoBackend::paint(PaintContext &context) {
  current_ = &context;
  cairo_t *cr = context.cr();
  Info info(this, 0, 0, context.width(), context.height());
  cairo_scale(cr, 1, 1);
  cairo_set_source_rgb(cr, 1, 1, 1);
  cairo_rectangle(cr, 0, 0, info.width(), info.height());
  cairo_fill(cr);
  graphics().root().accept(*this);
  cairo_stroke(cr);
  current_ = NULL;
}

void CairoBackend::visit_circle(wtk::Circle &that) {
  float x = info().left() + that.center().x().fix(info().width());
  float y = info().top() + that.center().y().fix(info().height());
  float r = that.radius().fix((info().width() + info().height()) / 2);
  cairo_t *cr = current().cr();
  cairo_set_source_rgb(cr, 0.0, 0.0, 0.5);
  cairo_set_line_width(cr, 1.0);
  cairo_arc(cr, x, y, r, 0, 2 * kPi);
}

void CairoBackend::visit_rect(wtk::Rect &that) {

}

} // namespace whiteout
