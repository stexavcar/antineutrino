#ifndef _VALUES_CHANNEL
#define _VALUES_CHANNEL

#include "utils/globals.h"
#include "utils/types.h"
#include "plankton/neutrino.h"

namespace neutrino {


// ---------------------
// --- C h a n n e l ---
// ---------------------

#define eChannelFields(VISIT, arg)                           \
  VISIT(String, name,         Name,        arg)                      \
  VISIT(Bool,   is_connected, IsConnected, arg)

class Channel : public Object {
public:
  typedef void (*Initializer)(IProxyConfiguration&);
  DECLARE_FIELD(void*, proxy);
  eChannelFields(DECLARE_OBJECT_FIELD, 0)

  Data *send(Runtime &runtime, String *name, Immediate *message);
  IObjectProxy *ensure_proxy(Runtime &runtime);

  static const uword kNameOffset        = Object::kHeaderSize;
  static const uword kIsConnectedOffset = kNameOffset + kPointerSize;
  static const uword kProxyOffset       = kIsConnectedOffset + kPointerSize;
  static const uword kSize              = kProxyOffset + kPointerSize;
private:
  static Initializer lookup_internal(string name);
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
  template <class C> static C *open(plankton::Value *obj);
  template <class C> static C wrap(Value *obj);
  static void *close(FImmediate *value);
  static plankton::Value new_value(ExtendedValueDTable &methods, void *origin);
  static plankton::Value new_value_from(plankton::Value *source, void *origin);
  static Data *send_message(Runtime &runtime, IObjectProxy &channel,
      String *name, Immediate *message);
};


/**
 * This d-table adds a few methods that we need internally.  Any
 * d-table used with values must be a subclass of this.
 */
struct ExtendedValueDTable : public plankton::Value::DTable {
  Data *(plankton::Value::*value_to_data_)();
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
