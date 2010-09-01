#include <cstdio>
#include <cairo-quartz.h>
#include <Carbon/Carbon.h>

#include "whiteout/cairo-backend.h"


namespace whiteout {


class CairoBackend::Data {
public:
  bool initialize(int width, int height);
  void open();
private:
  OSStatus repaint(EventHandlerCallRef call_ref, EventRef event);
  void tick();
  static OSStatus repaint_bridge(EventHandlerCallRef call_ref,
      EventRef event, void *data);
  static void tick_bridge(EventLoopTimerRef timer, void *data);
  WindowRef window() { return window_; }
  Rect rect() { return rect_; }
  WindowRef window_;
  Rect rect_;
};


CairoBackend::CairoBackend()
    : data_(new Data()) { }


void CairoBackend::initialize(int width, int height) {
  data()->initialize(width, height);
}


void CairoBackend::open() {
  data()->open();
}


OSStatus CairoBackend::Data::repaint(EventHandlerCallRef call_ref, EventRef event) {
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
//          whiteout::PaintContext paint_context(cr, width, height);
//          backend().paint(paint_context);
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

void CairoBackend::Data::tick() {
//  backend().graphics().animator().tick();
  Rect window_rect;
  SetRect(&window_rect, rect().left, rect().top, rect().right - rect().left,
      rect().bottom - rect().top);
  InvalWindowRect(window_, &window_rect);
}

OSStatus CairoBackend::Data::repaint_bridge(EventHandlerCallRef call_ref,
    EventRef event, void *data) {
  return static_cast<CairoBackend::Data*>(data)->repaint(call_ref, event);
}

void CairoBackend::Data::tick_bridge(EventLoopTimerRef timer, void *data) {
  static_cast<CairoBackend::Data*>(data)->tick();
}


bool CairoBackend::Data::initialize(int width, int height) {
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


void CairoBackend::Data::open() {
  ShowWindow(window());
  RunApplicationEventLoop();
}


} // namespace whiteout
