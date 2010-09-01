#include "platform/frame.h"
#include "utils/smart-ptrs-inl.h"

#include <cairo-quartz.h>
#include <Carbon/Carbon.h>

namespace neutrino {

class Frame::Data {
public:
  Data(FrameSettings &settings) : settings_(settings) { }
  Rect &rect() { return rect_; }
  WindowRef &window() { return window_; }
  bool open(Cairo &cairo);
  FrameSettings &settings() { return settings_; }
private:
  Rect rect_;
  WindowRef window_;
  FrameSettings &settings_;
};

bool Frame::Data::open(Cairo &cairo) {
  SetRect(&rect_, 30, 60, 30 + settings().width, 60 + settings().height);
  int attribs = kWindowStandardHandlerAttribute
      | kWindowAsyncDragAttribute | kWindowStandardDocumentAttributes
      | kWindowLiveResizeAttribute;
  OSStatus err = CreateNewWindow(kUtilityWindowClass, attribs, &rect_,
      &window_);
  if (err != noErr)
    return false;
  ShowWindow(window());
  RunApplicationEventLoop();
  return true;
}

Frame::Frame(FrameSettings &settings)
  : settings_(settings) { }

Frame::~Frame() { }

bool Frame::initialize() {
  data_.set(new Data(settings()));
  return true;
}

bool Frame::open(Cairo &cairo) {
  return data()->open(cairo);
}

} // namespace neutrino
