#include "utils/misc.h"
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
  cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
  cairo_set_line_width(cr, 3.0);
  graphics().root().accept(*this);
  cairo_stroke(cr);
  current_ = NULL;
}

void CairoBackend::visit_circle(wtk::Circle &that) {
  float x = info().left() + that.center().x().fix(info().width());
  float y = info().top() + that.center().y().fix(info().height());
  float r = that.radius().fix((info().width() + info().height()) / 2);
  cairo_arc(current().cr(), x, y, r, 0, 2 * kPi);
}

void CairoBackend::visit_rect(wtk::Rect &that) {
  Info &info = this->info();
  float left = info.top() + that.top_left().x().fix(info.width());
  float top = info.left() + that.top_left().y().fix(info.height());
  float width = that.size().width().fix(info.width());
  float height = that.size().height().fix(info.height());
  wtk::Quant &corner_radius = that.corner_radius();
  if (corner_radius.is_zero()) {
    cairo_rectangle(current().cr(), left, top, width, height);
  } else {
    int least = neutrino::min(info.width(), info.height());
    float r = corner_radius.fix(least);
    if (r > least / 4)
      r = least / 4;
    cairo_t *cr = current().cr();
    cairo_move_to(cr, left + r, top);
    cairo_line_to(cr, left + width - r, top);
    cairo_arc(cr, left + width - r, top + r, r, -kPi / 2, 0);
    cairo_line_to(cr, left + width, top + height - r);
    cairo_arc(cr, left + width - r, top + height - r, r, 0, kPi / 2);
    cairo_line_to(cr, left + r, top + height);
    cairo_arc(cr, left + r, top + height - r, r, kPi / 2, kPi);
    cairo_line_to(cr, left, top + r);
    cairo_arc(cr, left + r, top + r, r, kPi, -kPi / 2);
  }
}

} // namespace whiteout
