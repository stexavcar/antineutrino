#include <stdio.h>
#include "plankton/neutrino.h"
#include "platform/stdc-inl.h"

namespace plankton {

class TestNativesChannel : public neutrino::IExternalChannel {
public:
  virtual Value receive(neutrino::IMessage &value);
};

Value TestNativesChannel::receive(neutrino::IMessage &message) {
  Tuple args = cast<Tuple>(message.contents());
  Integer a_obj = cast<Integer>(args[0]);
  int a = a_obj.value();
  Integer b_obj = cast<Integer>(args[1]);
  int b = b_obj.value();
  int result = (a < b) ? a : b;
  return message.context().factory().new_integer(result);
}

SETUP_NEUTRINO_CHANNEL(test_natives)(neutrino::IExternalChannelConfiguration &config) {
  TestNativesChannel *channel = new TestNativesChannel();
  config.bind(*channel);
}

} // neutrino
