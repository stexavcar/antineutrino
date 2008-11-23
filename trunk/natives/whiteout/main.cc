#include "main/main.h"
#include "plankton/neutrino.h"
#include "platform/stdc-inl.h"
#include "whiteout/cairo-quartz.h"


class WhiteoutChannel : public neutrino::IObjectProxy {
public:
  virtual plankton::Value receive(neutrino::IMessage &value);
};


static bool start_cairo_quartz() {
  wtk::Container root(wtk::Point(0.25, 0.25), wtk::Size(0.5, 0.5));
  wtk::Rect rect(wtk::Point(0.0, 0.0), wtk::Size(1.0, 1.0));
  rect.corner_radius() = 0.02;
  root.add(rect);
  wtk::Circle circle(wtk::Point(0.5, 0.5), 0.9);
  root.add(circle);
  wtk::Text text(wtk::Point(0.5, 0.5), "Hello World");
  root.add(text);
  wtk::Graphics graphics(root);
  whiteout::CairoBackend backend(graphics);
  whiteout::CairoQuartz cairo_quartz(backend);
  if (!cairo_quartz.initialize(640, 480))
    return false;
  cairo_quartz.run();
  return true;
}


plankton::Value WhiteoutChannel::receive(neutrino::IMessage &message) {
  plankton::Tuple args = plankton::cast<plankton::Tuple>(message.contents());
  plankton::IBuilder &factory = message.context().factory();
  int operation = plankton::cast<plankton::Integer>(args[0]).value();
  switch (operation) {
    case 0:
      start_cairo_quartz();
      break;
  }
  return factory.get_void();
}
