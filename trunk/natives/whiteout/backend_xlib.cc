#include <cairo-xlib.h>
#include <X11/Xlib.h>
#include <cstdio>
#include "whiteout/backend.h"

namespace whiteout {


class XLibBackend : public Backend {
public:
  XLibBackend(CairoPainter &painter) : Backend(painter) { }
  virtual Frame *open(int width, int height);
};


class XLibFrame : public Frame {
public:
  XLibFrame(XLibBackend &backend, Display *display,
      cairo_surface_t *surface)
      : display_(display)
      , surface_(surface)
      , backend_(backend) { }
  void process_events();
  void repaint();
  Display *display() { return display_; }
  XLibBackend &backend() { return backend_; }
private:
  Display *display_;
  cairo_surface_t *surface_;
  XLibBackend &backend_;
};


Frame *XLibBackend::open(int width, int height) {
  // Create the window
  Display *display = XOpenDisplay(NULL);
  if (!display) return NULL;
  int screen = DefaultScreen(display);
  int black_color = BlackPixel(display, screen);
  Window window = XCreateSimpleWindow(display, DefaultRootWindow(display),
      0, 0, 200, 100, 0, black_color, black_color);
  Atom delete_atom = XInternAtom(display, "WM_DELETE_WINDOW", false);
  XSetWMProtocols(display, window, &delete_atom, false);
  XSelectInput(display, window, ExposureMask | SubstructureNotifyMask);
  XMapWindow(display, window);
  Visual *visual = DefaultVisual(display, screen);
  cairo_surface_t *surface = cairo_xlib_surface_create(display, window, visual, width, height);
  XLibFrame *frame = new XLibFrame(*this, display, surface);
  frame->process_events();
  return frame;
}


void XLibFrame::repaint() {
  cairo_t *cr = cairo_create(surface_);
  backend().painter().repaint(cr);
}


void XLibFrame::process_events() {
  while (true) {
    XEvent event;
    XNextEvent(display(), &event);
    switch (event.type) {
      case Expose:
        repaint();
        break;
    }
  }
}


static Backend *create_xlib_backend(CairoPainter &painter) {
  return new XLibBackend(painter);
}


static RegisterBackend register_xlib_backend(create_xlib_backend);


} // namespace whiteout
