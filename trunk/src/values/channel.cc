#include "values/values.h"
#include "io/image-inl.h"

namespace neutrino {


static const string kConfiguratorNamePattern = "configure_neptune_%_channel";


// ---------------------
// --- C h a n n e l ---
// ---------------------

Data *Channel::send(Runtime &runtime, Immediate *message) {
  IExternalChannel *proxy = ensure_proxy(runtime);
  if (proxy == NULL) return runtime.roots().vhoid();
  return ApiUtils::send_message(runtime, *proxy, message);
}


class ExternalChannelConfigurationImpl : public IExternalChannelConfiguration {
public:
  ExternalChannelConfigurationImpl();
  virtual void bind(IExternalChannel &channel);
  IExternalChannel *channel() { return channel_; }
private:
  IExternalChannel *channel_;
};


ExternalChannelConfigurationImpl::ExternalChannelConfigurationImpl()
  : channel_(NULL) { }


void ExternalChannelConfigurationImpl::bind(IExternalChannel &channel) {
  channel_ = &channel;
}


IExternalChannel *Channel::ensure_proxy(Runtime &runtime) {
  if (is<True>(is_connected()))
    return static_cast<IExternalChannel*>(proxy());
  set_is_connected(runtime.roots().thrue());
  DynamicLibraryCollection *dylibs = runtime.dylibs();
  if (dylibs == NULL) return NULL;
  string_buffer buf;
  buf.printf(kConfiguratorNamePattern, elms(name()));
  string name = buf.to_string();
  void *ptr = dylibs->lookup(name);
  if (ptr == NULL) return NULL;
  typedef void (*Initializer)(IExternalChannelConfiguration&);
  Initializer init = function_cast<Initializer>(ptr);
  ExternalChannelConfigurationImpl config;
  init(config);
  if (config.channel() == NULL) return NULL;
  set_proxy(config.channel());
  return config.channel();
}


// -----------------
// --- U t i l s ---
// -----------------

template <class C> C *ApiUtils::open(plankton::Value *obj) {
  return static_cast<C*>(obj->origin());
}


template <class C> C ApiUtils::wrap(Value *obj) {
  plankton::Value val = new_value(LiveValueDTableImpl::instance(), obj);
  return *reinterpret_cast<C*>(&val);
}


void *ApiUtils::close(FImmediate *obj) {
  return static_cast<void*>(obj);
}


plankton::Value ApiUtils::new_value(ExtendedValueDTable &dtable, void *origin) {
  return plankton::Value(dtable, origin);
}


plankton::Value ApiUtils::new_value_from(plankton::Value *source, void *origin) {
  return plankton::Value(source->dtable(), origin);
}


class BuilderImpl : public plankton::IBuilder {
public:
  BuilderImpl(Runtime &runtime) : runtime_(runtime) { }
  virtual plankton::Integer new_integer(word value);
  virtual plankton::String new_string(const char *data, unsigned length);
  virtual plankton::Null get_null();
  virtual plankton::Value new_raw_proxy(unsigned size);
private:
  Runtime &runtime() { return runtime_; }
  Runtime &runtime_;
};


class MessageContextImpl : public IMessageContext {
public:
  MessageContextImpl(plankton::IBuilder &factory) : factory_(factory) { }
  virtual plankton::IBuilder &factory() { return factory_; }
private:
  plankton::IBuilder &factory_;
};


class MessageImpl : public IMessage {
public:
  MessageImpl(plankton::Value contents, IMessageContext &context)
    : contents_(contents)
    , context_(context) { }
  virtual plankton::Value contents() { return contents_; }
  virtual IMessageContext &context() { return context_; }
private:
  plankton::Value contents_;
  IMessageContext &context_;
};


Data *ApiUtils::send_message(Runtime &runtime, IExternalChannel &channel,
    Immediate *contents) {
  plankton::Value value = new_value(LiveValueDTableImpl::instance(), contents);
  BuilderImpl factory(runtime);
  MessageContextImpl context(factory);
  MessageImpl message(value, context);
  plankton::Value result = channel.receive(message);
  ExtendedValueDTable &methods = static_cast<ExtendedValueDTable&>(result.dtable());
  return (result.*(methods.value_to_data_))();
}


plankton::Integer BuilderImpl::new_integer(word value) {
  Smi *obj = Smi::from_int(value);
  return ApiUtils::wrap<plankton::Integer>(obj);
}


plankton::String BuilderImpl::new_string(const char *data, unsigned length) {
  Data *str = runtime().heap().new_string(string(data, length));
  return ApiUtils::wrap<plankton::String>(cast<String>(str));
}


plankton::Null BuilderImpl::get_null() {
  Null *nuhll = runtime().roots().nuhll();
  return ApiUtils::wrap<plankton::Null>(nuhll);
}


plankton::Value BuilderImpl::new_raw_proxy(unsigned size) {
  Data *result = cast<Value>(runtime().heap().new_buffer(size));
  plankton::Value val = ApiUtils::new_value(LiveValueDTableImpl::instance(), result);
  return val;
}


// -----------------------------------------
// --- L i v e   V a l u e   D T a b l e ---
// -----------------------------------------

LiveValueDTableImpl LiveValueDTableImpl::instance_;

class LiveNValue : public plankton::Value {
public:
  plankton::Value::Tag type();
  void *proxy_deref(unsigned size);
  Data *to_data();
};

class LiveNInteger : public plankton::Integer {
public:
  int value();
};

class LiveNString : public plankton::String {
public:
  int length();
  char get(unsigned index);
  const char *c_str();
};

class LiveNTuple : public plankton::Tuple {
public:
  int length();
  plankton::Value get(unsigned index);
};

LiveValueDTableImpl::LiveValueDTableImpl() {
  value_type_ = static_cast<plankton::Value::Tag (plankton::Value::*)()>(&LiveNValue::type);
  integer_value_ = static_cast<int (plankton::Integer::*)()>(&LiveNInteger::value);
/*
  string_length_ = static_cast<int (NString::*)()>(&FrozenNString::length);
  string_get_ = static_cast<char (NString::*)(int)>(&FrozenNString::get);
  tuple_length_ = static_cast<int (NTuple::*)()>(&FrozenNTuple::length);
*/
  string_c_str_ = static_cast<const char *(plankton::String::*)()>(&LiveNString::c_str);
  tuple_get_ = static_cast<plankton::Value (plankton::Tuple::*)(unsigned)>(&LiveNTuple::get);
  proxy_deref_ = static_cast<void *(plankton::Value::*)(unsigned)>(&LiveNValue::proxy_deref);
  value_to_data_ = static_cast<Data *(plankton::Value::*)()>(&LiveNValue::to_data);
}

plankton::Value::Tag LiveNValue::type() {
  InstanceType type = ApiUtils::open<Immediate>(this)->type();
  switch (type) {
    case tString:
      return vtString;
    case tSmi:
      return vtInteger;
    case tTuple:
      return vtTuple;
    case tBuffer:
      return vtProxy;
    default:
      UNHANDLED(InstanceType, type);
      return vtUnknown;
  }
}

void *LiveNValue::proxy_deref(unsigned size) {
  Buffer *obj = ApiUtils::open<Buffer>(this);
  if (obj->size<byte>() == size) {
    return obj->buffer<byte>().start();
  } else {
    return 0;
  }
}

Data *LiveNValue::to_data() {
  return ApiUtils::open<Immediate>(this);
}

int LiveNInteger::value() {
  return ApiUtils::open<Smi>(this)->value();
}

const char *LiveNString::c_str() {
  return ApiUtils::open<neutrino::String>(this)->c_str().start();
}

plankton::Value LiveNTuple::get(unsigned index) {
  neutrino::Value *result = ApiUtils::open<neutrino::Tuple>(this)->get(index);
  return ApiUtils::new_value_from(this, result);
}


// ---------------------------------------------
// --- F r o z e n   V a l u e   D T a b l e ---
// ---------------------------------------------

FrozenValueDTableImpl FrozenValueDTableImpl::instance_;

class FrozenNValue : public plankton::Value {
public:
  plankton::Value::Tag type();
};

class FrozenNInteger : public plankton::Integer {
public:
  int value();
};

class FrozenNString : public plankton::String {
public:
  unsigned length();
  unsigned get(unsigned index);
  const char *c_str();
};

class FrozenNTuple : public plankton::Tuple {
public:
  int length();
  plankton::Value get(unsigned index);
};

FrozenValueDTableImpl::FrozenValueDTableImpl() {
  value_type_ = static_cast<plankton::Value::Tag (plankton::Value::*)()>(&FrozenNValue::type);
  integer_value_ = static_cast<int (plankton::Integer::*)()>(&FrozenNInteger::value);
  string_length_ = static_cast<unsigned (plankton::String::*)()>(&FrozenNString::length);
  string_get_ = static_cast<unsigned (plankton::String::*)(unsigned)>(&FrozenNString::get);
  string_c_str_ = static_cast<const char *(plankton::String::*)()>(&FrozenNString::c_str);
  tuple_length_ = static_cast<int (plankton::Tuple::*)()>(&FrozenNTuple::length);
  tuple_get_ = static_cast<plankton::Value (plankton::Tuple::*)(unsigned)>(&FrozenNTuple::get);
}

plankton::Value::Tag FrozenNValue::type() {
  InstanceType type = ApiUtils::open<FImmediate>(this)->type();
  switch (type) {
    case tString:
      return vtString;
    case tSmi:
      return vtInteger;
    case tTuple:
      return vtTuple;
    default:
      return vtUnknown;
  }
}

int FrozenNInteger::value() {
  return ApiUtils::open<FSmi>(this)->value();
}

unsigned FrozenNString::length() {
  return ApiUtils::open<FString>(this)->length();
}

unsigned FrozenNString::get(unsigned index) {
  return ApiUtils::open<FString>(this)->get(index);
}

const char *FrozenNString::c_str() {
  FString *str = ApiUtils::open<FString>(this);
  uword length = str->length();
  char *result = new char[length + 1];
  for (uword i = 0; i < length; i++)
    result[i] = str->get(i);
  result[length] = '\0';
  return result;
}

int FrozenNTuple::length() {
  return ApiUtils::open<FTuple>(this)->length();
}

plankton::Value FrozenNTuple::get(unsigned index) {
  FImmediate *result = ApiUtils::open<FTuple>(this)->at(index);
  return ApiUtils::new_value_from(this, result);
}

} // neutrino
