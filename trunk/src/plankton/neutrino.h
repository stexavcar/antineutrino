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


class RegisterInternalChannel {
public:
  typedef void (*ConfigCallback)(IExternalChannelConfiguration&);
  inline RegisterInternalChannel(string name, ConfigCallback callback)
      : name_(name)
      , callback_(callback)
      , prev_(top_) {
    top_ = this;
  }
private:
  friend class Channel;
  static RegisterInternalChannel *top() { return top_; }
  string name() { return name_; }
  ConfigCallback callback() { return callback_; }
  RegisterInternalChannel *prev() { return prev_; }
  string name_;
  ConfigCallback callback_;
  RegisterInternalChannel *prev_;
  static RegisterInternalChannel *top_;
};


} // neutrino

#endif // _PUBLIC_CHANNEL
