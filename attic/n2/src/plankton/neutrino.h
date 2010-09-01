#ifndef _PUBLIC_CHANNEL
#define _PUBLIC_CHANNEL


#include "plankton/cpp/channel.h"
#include <map>


namespace neutrino {


class IMessage;
class IMessageContext;
class IValueFactory;
class IObjectProxyDescriptor;


class IObjectProxy {
public:
  typedef plankton::Value (IObjectProxy::*method)(IMessage&);
  virtual IObjectProxyDescriptor &descriptor() = 0;
};


class IObjectProxyDescriptor {
public:
  virtual IObjectProxy::method get_method(plankton::String name, int argc) = 0;
};


class MappingObjectProxyDescriptor : public IObjectProxyDescriptor {
public:
  virtual IObjectProxy::method get_method(plankton::String name, int argc);
  template <typename T>
  void register_method(const char *name, int argc, plankton::Value (T::*method)(IMessage&));
private:
  class selector {
  public:
    selector(const char *name, int argc) : name_(name), argc_(argc) { }
    int operator<(const selector &b) const {
      int diff = argc_ - b.argc_;
      if (diff != 0) return diff;
      else return strcmp(name_, b.name_);
    }
  private:
    const char *name_;
    int argc_;
  };
  typedef std::map<selector, IObjectProxy::method> method_map;
  method_map &methods() { return methods_; }
  method_map methods_;
};


template <typename T>
void MappingObjectProxyDescriptor::register_method(const char *name,
    int argc, plankton::Value (T::*method)(IMessage&)) {
  methods()[selector(name, argc)] = static_cast<IObjectProxy::method>(method);
}


class MappingObjectProxy : public IObjectProxy {
public:
  virtual MappingObjectProxyDescriptor &descriptor();
private:
  MappingObjectProxyDescriptor descriptor_;
};


class IProxyConfiguration {
public:
  virtual void bind(IObjectProxy &channel) = 0;
};


class IMessage {
public:
  virtual plankton::Value operator[](int index) = 0;
  virtual IMessageContext &context() = 0;
};


class IMessageContext {
public:
  virtual plankton::IBuilder &factory() = 0;
};


class RegisterInternalChannel {
public:
  typedef void (callback_t)(neutrino::IProxyConfiguration&);
  RegisterInternalChannel(const char *name, callback_t *callback);
  const char *name() { return name_; }
  callback_t *callback() { return callback_; }
  RegisterInternalChannel *prev() { return prev_; }
  static RegisterInternalChannel *first() { return first_; }
private:
  const char *name_;
  callback_t *callback_;
  RegisterInternalChannel *prev_;
  static RegisterInternalChannel *first_;
};


} // neutrino

#endif // _PUBLIC_CHANNEL
