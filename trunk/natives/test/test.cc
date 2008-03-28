#include <stdio.h>
#include "public/neutrino.h"

namespace neutrino {

class TestNativesChannel : public IExternalChannel {
public:
  virtual NValue receive(IMessage &value);
};

NValue TestNativesChannel::receive(IMessage &message) {
  NTuple args = cast<NTuple>(message.contents());
  NInteger a_obj = cast<NInteger>(args[0]);
  int a = a_obj.value();
  NInteger b_obj = cast<NInteger>(args[1]);
  int b = b_obj.value();
  int result = (a < b) ? a : b;
  return message.context().factory().new_integer(result);
}

extern "C" void configure_neptune_test_natives_channel(IExternalChannelConfiguration &config) {
  TestNativesChannel *channel = new TestNativesChannel();
  config.bind(*channel);
}

} // neutrino
