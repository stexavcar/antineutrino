#include <stdio.h>
#include "plankton/neutrino.h"
#include "platform/stdc-inl.h"

namespace plankton {

class TestNativesChannel : public neutrino::MappingObjectProxy {
public:
  Value min(neutrino::IMessage &value);
};

Value TestNativesChannel::min(neutrino::IMessage &message) {
  Integer a_obj = cast<Integer>(message[0]);
  int a = a_obj.value();
  Integer b_obj = cast<Integer>(message[1]);
  int b = b_obj.value();
  int result = (a < b) ? a : b;
  return message.context().factory().new_integer(result);
}

SETUP_NEUTRINO_CHANNEL(test_natives)(neutrino::IProxyConfiguration &config) {
  TestNativesChannel *channel = new TestNativesChannel();
  neutrino::MappingObjectProxyDescriptor &desc = channel->descriptor();
  desc.register_method("min", 2, &TestNativesChannel::min);
  config.bind(*channel);
}

} // neutrino
