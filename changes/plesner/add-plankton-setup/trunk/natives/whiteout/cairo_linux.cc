#include "plankton/neutrino.h"
#include "platform/stdc-inl.h"
#include <cairo.h>

namespace plankton {

class CairoChannel : public neutrino::IExternalChannel {
public:
  virtual Value receive(neutrino::IMessage &value);
};

static void draw() {
  cairo_t *cr = 0;

  double xc = 128.0;
  double yc = 128.0;
  double radius = 100.0;
  double angle1 = 45.0  * (3.1415926/180.0);  /* angles are specified */
  double angle2 = 180.0 * (3.1415926/180.0);  /* in radians           */

  cairo_set_line_width (cr, 10.0);
  cairo_arc (cr, xc, yc, radius, angle1, angle2);
  cairo_stroke (cr);

  /* draw helping lines */
  cairo_set_source_rgba (cr, 1, 0.2, 0.2, 0.6);
  cairo_set_line_width (cr, 6.0);

  cairo_arc (cr, xc, yc, 10.0, 0, 2*3.1415926);
  cairo_fill (cr);

  cairo_arc (cr, xc, yc, radius, angle1, angle1);
  cairo_line_to (cr, xc, yc);
  cairo_arc (cr, xc, yc, radius, angle2, angle2);
  cairo_line_to (cr, xc, yc);
  cairo_stroke (cr);
}

Value CairoChannel::receive(neutrino::IMessage &message) {
  draw();
  return message.context().factory().get_null();
}

SETUP_NEUTRINO_CHANNEL(cairo)(neutrino::IExternalChannelConfiguration &config) {
  CairoChannel *cairo = new CairoChannel();
  config.bind(*cairo);
}

} // plankton
