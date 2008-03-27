#include <stdio.h>
#include "public/neutrino.h"

namespace neutrino {

class TestNativesChannel : public IExternalChannel {
public:
  virtual int receive(NValue value);
};

int TestNativesChannel::receive(NValue value) {
  NTuple args = cast<NTuple>(value);
  NInteger a_obj = cast<NInteger>(args[1]);
  int a = a_obj.value();
  NInteger b_obj = cast<NInteger>(args[2]);
  int b = b_obj.value();
  return (a < b) ? a : b;
}

extern "C" void configure_neptune_test_natives_channel(IExternalChannelConfiguration &config) {
  TestNativesChannel *channel = new TestNativesChannel();
  config.bind(*channel);
}

} // neutrino
