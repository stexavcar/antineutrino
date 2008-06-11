#ifndef _PUBLIC_CHANNEL
#define _PUBLIC_CHANNEL


#include "plankton/cpp/channel.h"


namespace neutrino {


class IMessage;
class IMessageContext;
class IValueFactory;


class IExternalChannel {
public:
  virtual plankton::Value receive(IMessage &message) = 0;
};


class IExternalChannelConfiguration {
public:
  virtual void bind(IExternalChannel &channel) = 0;
};


class IMessage {
public:
  virtual plankton::Value contents() = 0;
  virtual IMessageContext &context() = 0;
};


class IMessageContext {
public:
  virtual plankton::IBuilder &factory() = 0;
};


} // neutrino

#endif // _PUBLIC_CHANNEL
