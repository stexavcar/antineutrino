#ifndef _WHITEOUT_CAIRO_QUARTZ
#define _WHITEOUT_CAIRO_QUARTZ

#include <Carbon/Carbon.h>

#include "whiteout/wtk.h"
#include "whiteout/cairo-backend.h"

namespace whiteout {

class CairoQuartz {
public:
  CairoQuartz(whiteout::CairoBackend &backend)
    : backend_(backend) { }
  bool initialize(int width, int height);
  void run();
private:
  OSStatus repaint(EventHandlerCallRef call_ref, EventRef event);
  void tick();
  static OSStatus repaint_bridge(EventHandlerCallRef call_ref,
    EventRef event, void *data);
  static void tick_bridge(EventLoopTimerRef timer, void *data);
  whiteout::CairoBackend &backend() { return backend_; }
  WindowRef window() { return window_; }
  Rect rect() { return rect_; }
  whiteout::CairoBackend &backend_;
  WindowRef window_;
  Rect rect_;
};

} // namespace whiteout

#endif // _WHITEOUT_CAIRO_QUARTZ
