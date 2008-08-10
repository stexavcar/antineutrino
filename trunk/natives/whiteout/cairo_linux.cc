#include <cairo.h>
#include <cstdio>
#include "plankton/neutrino.h"
#include "platform/stdc-inl.h"
#include "whiteout/backend.h"

namespace whiteout {


class CairoChannelPainter : public CairoPainter {
public:
  virtual void repaint(cairo_t *ct);
};


class CairoChannel : public neutrino::IExternalChannel {
public:
  explicit CairoChannel(Backend *backend);
  virtual ~CairoChannel();
  virtual plankton::Value receive(neutrino::IMessage &value);

  void do_open_frame(int width, int height);
  plankton::Value do_alloc(plankton::IBuilder &builder, int type);
private:
  Backend *backend_;
  Frame *frame_;
  static const int ccOpenFrame = 0;
  static const int ccAlloc = 1;

  static const int etCircle = 0;
};


void CairoChannel::do_open_frame(int width, int height) {
  frame_ = backend_->open(width, height);
}


plankton::Value CairoChannel::do_alloc(plankton::IBuilder &builder, int type) {
  switch (type) {
    default:
      return builder.get_null();
  }
}


plankton::Value CairoChannel::receive(neutrino::IMessage &message) {
  plankton::Tuple args = plankton::cast<plankton::Tuple>(message.contents());
  switch (plankton::cast<plankton::Integer>(args[0]).value()) {
    case ccOpenFrame: {
      int width = plankton::cast<plankton::Integer>(args[1]).value();
      int height = plankton::cast<plankton::Integer>(args[2]).value();
      do_open_frame(width, height);
      break;
    }
    case ccAlloc: {
      int type = plankton::cast<plankton::Integer>(args[1]).value();
      return do_alloc(message.context().factory(), type);
    }
  }
  return message.context().factory().get_void();
}


CairoChannel::CairoChannel(Backend *backend) : backend_(backend) {

}


CairoChannel::~CairoChannel() {
  delete backend_;
}


void CairoChannelPainter::repaint(cairo_t *cr) {
  double xc = 128.0;
  double yc = 128.0;
  double radius = 100.0;
  double angle1 = 45.0  * (3.1415926/180.0);  // angles are specified
  double angle2 = 180.0 * (3.1415926/180.0);  // in radians

  cairo_set_line_width (cr, 10.0);
  cairo_arc (cr, xc, yc, radius, angle1, angle2);
  cairo_stroke (cr);

  // draw helping lines
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


SETUP_NEUTRINO_CHANNEL(cairo)(neutrino::IExternalChannelConfiguration &config) {
  CairoChannelPainter *painter = new CairoChannelPainter();
  Backend *backend = Backend::create(*painter);
  if (!backend) return;
  CairoChannel *cairo = new CairoChannel(backend);
  config.bind(*cairo);
}


} // plankton
