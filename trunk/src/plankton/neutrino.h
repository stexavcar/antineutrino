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
  virtual IObjectProxy::method get_method(plankton::String name) = 0;
};


class MappingObjectProxyDescriptor : public IObjectProxyDescriptor {
public:
  virtual IObjectProxy::method get_method(plankton::String name);
  template <typename T>
  void register_method(const char *name, plankton::Value (T::*method)(IMessage&));
private:
  class cmp_string {
  public:
    cmp_string(const char *str) : str_(str) { }
    int operator<(const cmp_string &b) const { return strcmp(str_, b.str_); }
  private:
    const char *str_;
  };
  typedef std::map<cmp_string, IObjectProxy::method> method_map;
  method_map &methods() { return methods_; }
  method_map methods_;
};


template <typename T>
void MappingObjectProxyDescriptor::register_method(const char *name,
    plankton::Value (T::*method)(IMessage&)) {
  methods()[name] = static_cast<IObjectProxy::method>(method);
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
  virtual plankton::Value contents() = 0;
  virtual IMessageContext &context() = 0;
};


class IMessageContext {
public:
  virtual plankton::IBuilder &factory() = 0;
};


} // neutrino

#endif // _PUBLIC_CHANNEL
