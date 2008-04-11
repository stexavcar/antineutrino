#ifndef _VALUES_CHANNEL
#define _VALUES_CHANNEL

#include "utils/globals.h"
#include "utils/types.h"
#include "public/neutrino.h"

namespace neutrino {


// ---------------------
// --- C h a n n e l ---
// ---------------------

#define eChannelFields(VISIT, arg)                           \
  VISIT(String, name,         Name,        arg)                      \
  VISIT(Bool,   is_connected, IsConnected, arg)

class Channel : public Object {
public:
  DECLARE_FIELD(void*, proxy);
  eChannelFields(DECLARE_OBJECT_FIELD, 0)
  
  Data *send(Runtime &runtime, Immediate *message);
  IExternalChannel *ensure_proxy(Runtime &runtime);

  static const uword kNameOffset        = Object::kHeaderSize;
  static const uword kIsConnectedOffset = kNameOffset + kPointerSize;
  static const uword kProxyOffset       = kIsConnectedOffset + kPointerSize;
  static const uword kSize              = kProxyOffset + kPointerSize;
};

template <> class ref_traits<Channel> : public ref_traits<Object> {
public:
  eChannelFields(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(Channel);


// -----------------
// --- U t i l s ---
// -----------------


class ApiUtils {
public:
  template <class C> static C *open(NValue *obj);
  template <class C> static C wrap(Value *obj);
  static void *close(FImmediate *value);
  static NValue new_value(ExtendedValueDTable &methods, void *origin);  
  static NValue new_value_from(NValue *source, void *origin);
  static Data *send_message(Runtime &runtime, IExternalChannel &channel,
      Immediate *message);
};


/**
 * This d-table adds a few methods that we need internally.  Any
 * d-table used with values must be a subclass of this.
 */
struct ExtendedValueDTable : public ValueDTable {
  Data *(NValue::*value_to_data_)();
};


class FrozenValueDTableImpl : public ExtendedValueDTable {
public:
  static ExtendedValueDTable &instance() { return instance_; }
private:
  FrozenValueDTableImpl();
  static FrozenValueDTableImpl instance_;
};


class LiveValueDTableImpl : public ExtendedValueDTable {
public:
  static ExtendedValueDTable &instance() { return instance_; }
private:
  LiveValueDTableImpl();
  static LiveValueDTableImpl instance_;
};


} // neutrino

#endif // _VALUES_CHANNEL
