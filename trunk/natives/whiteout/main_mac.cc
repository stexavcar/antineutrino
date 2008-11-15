#include <cstdio>
#include <cairo-quartz.h>
#include <Carbon/Carbon.h>


#include "whiteout/cairo-backend.h"

WindowRef gTestWindow;
Rect gRect;
int width = 640;
int height = 480;
int count = 0;

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

pascal void RedrawTimerCallback(EventLoopTimerRef theTimer, void* userData) {
  Rect windowRect;
  SetRect(&windowRect, 0, 0, width, height);
  InvalWindowRect(gTestWindow, &windowRect);
}

int main(int argc, char *argv[]) {
  SetRect(&gRect, 30, 60, 30 + width, 60 + height);
  OSStatus err = CreateNewWindow(kPlainWindowClass,
      kWindowStandardHandlerAttribute | kWindowAsyncDragAttribute,
      &gRect,
      &gTestWindow);
  if (err != noErr)
    return 1;

  const EventTypeSpec windowEvents[] = {
    { kEventClassWindow,    kEventWindowClickContentRgn},
    { kEventClassWindow,    kEventWindowDrawContent}
  };
  EventHandlerUPP gTestWindowEventProc = NewEventHandlerUPP(TestWindowEventHandler);
  err = InstallWindowEventHandler(gTestWindow,
      gTestWindowEventProc, GetEventTypeCount(windowEvents),
      windowEvents, gTestWindow, NULL);
  if (err != noErr)
    return 1;

  EventLoopTimerRef redrawTimer;
  InstallEventLoopTimer(GetMainEventLoop(),
      0.0,
      0.1 * kEventDurationSecond,
      NewEventLoopTimerUPP(RedrawTimerCallback),
      NULL,
      &redrawTimer);

  ShowWindow(gTestWindow);
  RunApplicationEventLoop();
  return 0;

}
