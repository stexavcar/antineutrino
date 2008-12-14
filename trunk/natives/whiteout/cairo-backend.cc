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
  cairo_translate(cr, 0, context.height());
  cairo_scale(cr, 1.0, -1.0);
  cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
  cairo_rectangle(cr, 0, 0, info.width(), info.height());
  cairo_fill(cr);
  cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
  cairo_set_line_width(cr, 2.0);
  graphics().root().accept(*this);
  current_ = NULL;
}

void CairoBackend::visit_circle(wtk::Circle &that) {
  Info &info = this->info();
  float x = info.left() + that.center().x().fix(info.width());
  float y = info.top() + that.center().y().fix(info.height());
  float least = neutrino::min(info.width(), info.height());
  float r = that.radius().fix(least / 2);
  cairo_t *cr = current().cr();
  cairo_arc(cr, x, y, r, 0, 2 * kPi);
  cairo_stroke(cr);
}

void CairoBackend::visit_rect(wtk::Rect &that) {
  Info &info = this->info();
  float left = info.left() + that.top_left().x().fix(info.width());
  float top = info.top() + that.top_left().y().fix(info.height());
  float width = that.size().width().fix(info.width());
  float height = that.size().height().fix(info.height());
  cairo_t *cr = current().cr();
  wtk::Quant &corner_radius = that.corner_radius();
  if (corner_radius.is_zero()) {
    cairo_rectangle(current().cr(), left, top, width, height);
  } else {
    int least = neutrino::min(info.width(), info.height());
    float r = corner_radius.fix(least);
    if (r > least / 4)
      r = least / 4;
    cairo_save(cr);
    cairo_move_to(cr, left + r, top);
    cairo_line_to(cr, left + width - r, top);
    cairo_arc(cr, left + width - r, top + r, r, -kPi / 2, 0);
    cairo_line_to(cr, left + width, top + height - r);
    cairo_arc(cr, left + width - r, top + height - r, r, 0, kPi / 2);
    cairo_line_to(cr, left + r, top + height);
    cairo_arc(cr, left + r, top + height - r, r, kPi / 2, kPi);
    cairo_line_to(cr, left, top + r);
    cairo_arc(cr, left + r, top + r, r, kPi, -kPi / 2);
    cairo_stroke(cr);
    cairo_restore(cr);
  }
}

void CairoBackend::visit_text(wtk::Text &that) {
  Info &info = this->info();
  cairo_t *cr = current().cr();
  float left = info.left() + that.top_left().x().fix(info.width());
  float top = info.top() + that.top_left().y().fix(info.height());
  cairo_save(cr);
  cairo_move_to(cr, left, top);
  neutrino::string str = that.value();
  cairo_show_text(cr, str.chars().start());
  cairo_restore(cr);
}

void CairoBackend::visit_container(wtk::Container &that) {
  Info &info = this->info();
  float left = info.left() + that.top_left().x().fix(info.width());
  float top = info.top() + that.top_left().y().fix(info.height());
  float width = that.size().width().fix(info.width());
  float height = that.size().height().fix(info.height());
  Info inner(this, left, top, width, height);
  for (unsigned i = 0; i < that.children().length(); i++)
    that.children()[i]->accept(*this);
}

} // namespace whiteout
