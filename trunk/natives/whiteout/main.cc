#include "main/main.h"
#include "plankton/neutrino.h"
#include "platform/stdc-inl.h"


class WhiteoutChannel : public neutrino::IObjectProxy {
public:
  virtual plankton::Value receive(neutrino::IMessage &value);
};


plankton::Value WhiteoutChannel::receive(neutrino::IMessage &message) {
  plankton::Tuple args = plankton::cast<plankton::Tuple>(message.contents());
  plankton::IBuilder &factory = message.context().factory();
  int operation = plankton::cast<plankton::Integer>(args[0]).value();
  switch (operation) {
    case 0:
      break;
  }
  return factory.get_void();
}
