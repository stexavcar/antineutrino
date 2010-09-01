#include "platform/frame.h"
#include "utils/check-inl.h"
#include "utils/log.h"
#include "utils/smart-ptrs-inl.h"

#include <X11/Xlib.h>
#include <cairo-xlib.h>
#include <unistd.h>

namespace neutrino {

template <>
class resource_cleaner<Display*> {
public:
  static void release(Display *display) {
    if (display) XCloseDisplay(display);
  }
};

template <>
class resource_cleaner<cairo_t*> {
public:
  static void release(cairo_t *cr) {
    if (cr) cairo_destroy(cr);
  }
};

template <>
class resource_cleaner<cairo_surface_t*> {
public:
  static void release(cairo_surface_t *surface) {
    if (surface) cairo_surface_destroy(surface);
  }
};

class Frame::Data {
public:
  Data(Display *display, Window window)
    : display_(display), window_(window) { }
  ~Data() { }
  Display *display() { return *display_; }
  Window window() { return window_; }
private:
  own_resource<Display*> display_;
  Window window_;
};

Frame::Frame(FrameSettings &settings)
  : settings_(settings) { }

Frame::~Frame() { }

bool Frame::initialize() {
  own_resource<Display*> display(XOpenDisplay(NULL));
  if (*display == NULL) return false;
  int blackColor = BlackPixel(*display, DefaultScreen(*display));
  Window window = XCreateSimpleWindow(*display, DefaultRootWindow(*display),
      0, 0, settings().width, settings().height, 0, blackColor, blackColor);
  data_.set(new Data(display.release(), window));
  return true;
}

bool Frame::open(Cairo &cairo) {
  Display *display = data()->display();
  Window window = data()->window();
  XSelectInput(display, window, ExposureMask | StructureNotifyMask);
  XMapWindow(display, window);
  while (true) {
    XEvent e;
    XNextEvent(display, &e);
    if (e.type == Expose) {
      own_resource<cairo_surface_t*> surface(cairo_xlib_surface_create(
          display, window, DefaultVisual(display, DefaultScreen(display)),
          settings().width, settings().height));
      assert *surface;
      own_resource<cairo_t*> resource(cairo_create(*surface));
      assert *resource;
      PaintContext context(settings().width, settings().height, *resource);
      cairo.paint(context);
    }
  }
  sleep(10);
  return true;
}

} // namespace neutrino
