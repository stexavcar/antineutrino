#ifndef _PLATFORM_FRAME
#define _PLATFORM_FRAME

#include "plankton/plankton.h"
#include "utils/global.h"
#include "utils/smart-ptrs.h"
#include "platform/cairo.h"

namespace neutrino {

struct FrameSettings {
  FrameSettings() : width(300), height(200) { }
  word width;
  word height;
};

class Frame {
public:
  class Data;
  Frame(FrameSettings &settings);
  virtual ~Frame();
  virtual bool initialize();
  virtual bool open(Cairo &cairo);
private:
  Data *data() { return *data_; }
  FrameSettings &settings() { return settings_; }
  own_ptr<Data> data_;
  FrameSettings &settings_;
};

} // namespace neutrino

#endif // _PLATFORM_FRAME
