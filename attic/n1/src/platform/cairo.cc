#include "platform/cairo.h"

namespace neutrino {

void Cairo::paint(PaintContext &context) {
  cairo_t *cr = context.cairo();
  cairo_set_source_rgba(cr, 0.0, 1.0, 0.0, 0.5);
  cairo_rectangle(cr, 0, 0, context.width() / 3, context.height() / 3);
  cairo_fill(cr);
  cairo_set_line_width(cr, 6.0);
  cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 0.5);
  cairo_arc(cr, 100, 100, 50, 0, 3.1415 * 2);
  cairo_stroke(cr);
}

} // namespace neutrino
