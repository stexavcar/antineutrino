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
  buf.printf(kConfiguratorNamePattern, name());
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

template <class C> C *ApiUtils::open(NValue *obj) {
  return static_cast<C*>(obj->origin());
}

void *ApiUtils::close(FImmediate *obj) {
  return static_cast<void*>(obj);
}

NValue ApiUtils::new_value(ValueDTable &methods, void *origin) {
  return NValue(methods, origin);
}


NValue ApiUtils::new_value(NValue *source, void *origin) {
  return NValue(source->methods(), origin);
}


Data *ApiUtils::send_message(Runtime &runtime, IExternalChannel &channel,
    Immediate *message) {
  NValue value = new_value(LiveValueDTableImpl::instance(), message);
  word result = channel.receive(value);
  return Smi::from_int(result);
}


// -----------------------------------------
// --- L i v e   V a l u e   D T a b l e ---
// -----------------------------------------

LiveValueDTableImpl LiveValueDTableImpl::instance_;

class LiveNValue : public NValue {
public:
  ValueType type();
};

class LiveNInteger : public NInteger {
public:
  int value();
};

class LiveNString : public NString {
public:
  int length();
  char get(int index);
  const char *c_str();
};

class LiveNTuple : public NTuple {
public:
  int length();
  NValue get(int index);
};

LiveValueDTableImpl::LiveValueDTableImpl() {
  value_type_ = static_cast<ValueType (NValue::*)()>(&LiveNValue::type);
  integer_value_ = static_cast<int (NInteger::*)()>(&LiveNInteger::value);
/*
  string_length_ = static_cast<int (NString::*)()>(&FrozenNString::length);
  string_get_ = static_cast<char (NString::*)(int)>(&FrozenNString::get);
  string_c_str_ = static_cast<const char *(NString::*)()>(&FrozenNString::c_str);
  tuple_length_ = static_cast<int (NTuple::*)()>(&FrozenNTuple::length);
*/
  tuple_get_ = static_cast<NValue (NTuple::*)(int)>(&LiveNTuple::get);
}

ValueType LiveNValue::type() {
  InstanceType type = ApiUtils::open<Immediate>(this)->type();
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

int LiveNInteger::value() {
  return ApiUtils::open<Smi>(this)->value();
}

NValue LiveNTuple::get(int index) {
  Value *result = ApiUtils::open<Tuple>(this)->get(index);
  return ApiUtils::new_value(this, result);
}


// ---------------------------------------------
// --- F r o z e n   V a l u e   D T a b l e ---
// ---------------------------------------------

FrozenValueDTableImpl FrozenValueDTableImpl::instance_;

class FrozenNValue : public NValue {
public:
  ValueType type();
};

class FrozenNInteger : public NInteger {
public:
  int value();
};

class FrozenNString : public NString {
public:
  int length();
  char get(int index);
  const char *c_str();
};

class FrozenNTuple : public NTuple {
public:
  int length();
  NValue get(int index);
};

FrozenValueDTableImpl::FrozenValueDTableImpl() {
  value_type_ = static_cast<ValueType (NValue::*)()>(&FrozenNValue::type);
  integer_value_ = static_cast<int (NInteger::*)()>(&FrozenNInteger::value);
  string_length_ = static_cast<int (NString::*)()>(&FrozenNString::length);
  string_get_ = static_cast<char (NString::*)(int)>(&FrozenNString::get);
  string_c_str_ = static_cast<const char *(NString::*)()>(&FrozenNString::c_str);
  tuple_length_ = static_cast<int (NTuple::*)()>(&FrozenNTuple::length);
  tuple_get_ = static_cast<NValue (NTuple::*)(int)>(&FrozenNTuple::get);
}

ValueType FrozenNValue::type() {
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

int FrozenNString::length() {
  return ApiUtils::open<FString>(this)->length();
}

char FrozenNString::get(int index) {
  return ApiUtils::open<FString>(this)->at(index);
}

const char *FrozenNString::c_str() {
  FString *str = ApiUtils::open<FString>(this);
  uword length = str->length();
  char *result = new char[length + 1];
  for (uword i = 0; i < length; i++)
    result[i] = str->at(i);
  result[length] = '\0';
  return result;
}

int FrozenNTuple::length() {
  return ApiUtils::open<FTuple>(this)->length();
}

NValue FrozenNTuple::get(int index) {
  FImmediate *result = ApiUtils::open<FTuple>(this)->at(index);
  return ApiUtils::new_value(this, result);
}

} // neutrino
