#include <cstdio>
#include <cairo-quartz.h>
#include <Carbon/Carbon.h>


#include "whiteout/cairo-backend.h"

/*
static pascal OSStatus TestWindowEventHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData ) {
    OSStatus    err     = eventNotHandledErr;
    UInt32  eventKind       = GetEventKind(inEvent);
    UInt32  eventClass      = GetEventClass(inEvent);
    WindowRef   window      = (WindowRef)inUserData;


    switch (eventClass)
    {
    case kEventClassWindow:
        if (eventKind == kEventWindowDrawContent)
        {
        CGContextRef    context;
        cairo_t     *cr;
        cairo_surface_t *surface;
        int     width, height;


        width = gRect.right - gRect.left;
        height = gRect.bottom - gRect.top;

        QDBeginCGContext(GetWindowPort(window), &context);

        surface = cairo_quartz_surface_create_for_cg_context(context, width, height);
        cr = cairo_create(surface);


        // Draw one of the Cairo snippets
        cairo_scale(cr, width/1.0, height/1.0);
        cairo_rectangle(cr, 0,0, 1.0, 1.0);
        cairo_set_source_rgb(cr, 1,1,1);
        cairo_fill(cr);
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        cairo_set_line_width(cr, 0.04);

            cairo_text_extents_t extents;

            const char *utf8 = "cairo";
            double x,y;

            cairo_select_font_face(cr, "Times",
                       CAIRO_FONT_SLANT_NORMAL,
                       CAIRO_FONT_WEIGHT_NORMAL);

            count = (count + 1) % 10;
            cairo_set_font_size(cr, 0.4 + (count / 100.0));
            cairo_text_extents(cr, utf8, &extents);

            x=0.1;
            y=0.6;

            cairo_move_to(cr, x,y);
            cairo_show_text(cr, utf8);

            // draw helping lines
            cairo_set_source_rgba(cr, 1,0.2,0.2,0.6);
            cairo_arc(cr, x, y, 0.05, 0, 2*M_PI);
            cairo_fill(cr);
            cairo_move_to(cr, x,y);
            cairo_rel_line_to(cr, 0, -extents.height);
            cairo_rel_line_to(cr, extents.width, 0);
            cairo_rel_line_to(cr, extents.x_bearing, -extents.y_bearing);
            cairo_stroke(cr);

        cairo_destroy(cr);

        CGContextFlush(context);

        QDEndCGContext(GetWindowPort(window), &context);


        err = noErr;
        }
        else if (eventKind == kEventWindowClickContentRgn)
        {
        Point   mouseLoc;

        GetGlobalMouse(&mouseLoc);
        DragWindow(window, mouseLoc, NULL);
        }
        break;
    }

    return err;
}
*/

class CairoQuartz {
public:
  CairoQuartz(int width, int height, whiteout::CairoBackend &backend)
    : width_(width), height_(height), backend_(backend) { }
  bool initialize();
  void run();
private:
  OSStatus repaint(EventHandlerCallRef call_ref, EventRef event);
  void tick();
  static OSStatus repaint_bridge(EventHandlerCallRef call_ref,
    EventRef event, void *data);
  static void tick_bridge(EventLoopTimerRef timer, void *data);
  int width() { return width_; }
  int height() { return height_; }
  whiteout::CairoBackend &backend() { return backend_; }
  WindowRef window() { return window_; }
  Rect rect() { return rect_; }
  int width_;
  int height_;
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
        case kEventWindowClose: {
          QuitAppModalLoopForWindow(window());
          return noErr;
        }
        case kEventWindowClickContentRgn: {
          Point mouse_loc;
          GetGlobalMouse(&mouse_loc);
          DragWindow(window(), mouse_loc, NULL);
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
  SetRect(&window_rect, 0, 0, width_, height_);
  InvalWindowRect(window_, &window_rect);
}

OSStatus CairoQuartz::repaint_bridge(EventHandlerCallRef call_ref,
    EventRef event, void *data) {
  return static_cast<CairoQuartz*>(data)->repaint(call_ref, event);
}

void CairoQuartz::tick_bridge(EventLoopTimerRef timer, void *data) {
  static_cast<CairoQuartz*>(data)->tick();
}


bool CairoQuartz::initialize() {
  SetRect(&rect_, 30, 60, 30 + width_, 60 + height_);

  OSStatus err = CreateNewWindow(kDocumentWindowClass,
      kWindowCloseBoxAttribute | kWindowStandardHandlerAttribute
      | kWindowAsyncDragAttribute,
      &rect_,
      &window_);
  if (err != noErr)
    return false;

  const EventTypeSpec windowEvents[] = {
    { kEventClassWindow, kEventWindowClickContentRgn},
    { kEventClassWindow, kEventWindowDrawContent},
    { kEventClassWindow, kEventWindowClose }
  };
  EventHandlerUPP gTestWindowEventProc = NewEventHandlerUPP(repaint_bridge);
  err = InstallWindowEventHandler(window_,
      gTestWindowEventProc, GetEventTypeCount(windowEvents),
      windowEvents, this, NULL);

   EventLoopTimerRef redraw_timer;
   InstallEventLoopTimer(GetMainEventLoop(),
       0.0,
       0.03 * kEventDurationSecond,
       NewEventLoopTimerUPP(tick_bridge),
       this,
       &redraw_timer);

   return err == noErr;
}


void CairoQuartz::run() {
  ShowWindow(window());
  RunAppModalLoopForWindow(window());
}


int main(int argc, char *argv[]) {
  wtk::Container root(wtk::Point(0.25, 0.25), wtk::Size(0.5, 0.5));
  wtk::Rect rect(wtk::Point(0.0, 0.0), wtk::Size(1.0, 1.0));
  rect.corner_radius() = 0.02;
  root.add(rect);
  wtk::Circle circle(wtk::Point(0.5, 0.5), 0.9);
  root.add(circle);
  wtk::Graphics graphics(root);
  whiteout::CairoBackend backend(graphics);
  CairoQuartz cairo_quartz(640, 480, backend);
  if (!cairo_quartz.initialize())
    return 1;
  cairo_quartz.run();
  return 0;
}
