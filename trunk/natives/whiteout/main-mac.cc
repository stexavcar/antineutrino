#include <cstdio>
#include <cairo-quartz.h>
#include <Carbon/Carbon.h>


#include "whiteout/cairo-backend.h"

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

OSStatus CairoQuartz::repaint(EventHandlerCallRef call_ref, EventRef event) {
  UInt32 event_kind = GetEventKind(event);
  UInt32 event_class = GetEventClass(event);
  switch (event_class) {
    case kEventClassWindow:
      switch (event_kind) {
        case kEventWindowDrawContent: {
          int width = rect().right - rect().left;
          int height = rect().bottom - rect().top;
          CGContextRef context;
          QDBeginCGContext(GetWindowPort(window()), &context);
          cairo_surface_t *surface = cairo_quartz_surface_create_for_cg_context(context, width, height);
          cairo_t *cr = cairo_create(surface);
          whiteout::PaintContext paint_context(cr, width, height);
          backend().paint(paint_context);
          cairo_destroy(cr);
          CGContextFlush(context);
          QDEndCGContext(GetWindowPort(window()), &context);
          return noErr;
        }
        case kEventWindowBoundsChanged: {
          GetEventParameter(event, kEventParamCurrentBounds, typeQDRectangle,
              NULL, sizeof(Rect), NULL, &rect_);
          return noErr;
        }
        case kEventWindowClose: {
          QuitApplicationEventLoop();
          return noErr;
        }
        break;
      }
  }
  return eventNotHandledErr;
}

void CairoQuartz::tick() {
  backend().graphics().animator().tick();
  Rect window_rect;
  SetRect(&window_rect, rect().left, rect().top, rect().right - rect().left,
      rect().bottom - rect().top);
  InvalWindowRect(window_, &window_rect);
}

OSStatus CairoQuartz::repaint_bridge(EventHandlerCallRef call_ref,
    EventRef event, void *data) {
  return static_cast<CairoQuartz*>(data)->repaint(call_ref, event);
}

void CairoQuartz::tick_bridge(EventLoopTimerRef timer, void *data) {
  static_cast<CairoQuartz*>(data)->tick();
}


bool CairoQuartz::initialize(int width, int height) {
  SetRect(&rect_, 30, 60, 30 + width, 60 + height);

  int attribs = kWindowStandardHandlerAttribute
    | kWindowAsyncDragAttribute | kWindowStandardDocumentAttributes
    | kWindowLiveResizeAttribute;
  OSStatus err = CreateNewWindow(kUtilityWindowClass, attribs, &rect_,
      &window_);
  if (err != noErr)
    return false;

  const EventTypeSpec window_events[] = {
    { kEventClassWindow, kEventWindowDrawContent},
    { kEventClassWindow, kEventWindowClose },
    { kEventClassWindow, kEventWindowBoundsChanged }
  };
  EventHandlerUPP event_proc = NewEventHandlerUPP(repaint_bridge);
  err = InstallWindowEventHandler(window(), event_proc,
      GetEventTypeCount(window_events), window_events, this, NULL);

  EventLoopTimerRef redraw_timer;
  InstallEventLoopTimer(GetMainEventLoop(), 0.0, kEventDurationSecond / 24,
      NewEventLoopTimerUPP(tick_bridge), this, &redraw_timer);
  return err == noErr;
}


void CairoQuartz::run() {
  ShowWindow(window());
  RunApplicationEventLoop();
}


int main(int argc, char *argv[]) {
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
  CairoQuartz cairo_quartz(backend);
  if (!cairo_quartz.initialize(640, 480))
    return 1;
  cairo_quartz.run();
  return 0;
}
