#ifndef _VALUES_CHANNEL
#define _VALUES_CHANNEL

#include "utils/globals.h"
#include "utils/types.h"
#include "public/neutrino.h"

namespace neutrino {


// ---------------------
// --- C h a n n e l ---
// ---------------------

#define FOR_EACH_CHANNEL_FIELD(VISIT, arg)                           \
  VISIT(String, name,         Name,        arg)                      \
  VISIT(Bool,   is_connected, IsConnected, arg)

class Channel : public Object {
public:
  DECLARE_FIELD(void*, proxy);
  FOR_EACH_CHANNEL_FIELD(DECLARE_OBJECT_FIELD, 0)
  
  Data *send(Runtime &runtime, Immediate *message);
  IExternalChannel *ensure_proxy(Runtime &runtime);

  static const uword kNameOffset        = Object::kHeaderSize;
  static const uword kIsConnectedOffset = kNameOffset + kPointerSize;
  static const uword kProxyOffset       = kIsConnectedOffset + kPointerSize;
  static const uword kSize              = kProxyOffset + kPointerSize;
};

template <> class ref_traits<Channel> : public ref_traits<Object> {
public:
  FOR_EACH_CHANNEL_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(Channel);


// -----------------
// --- U t i l s ---
// -----------------


class ApiUtils {
public:
  template <class C> static C *open(NValue *obj);
  static void *close(FImmediate *value);
  static NValue new_value(ValueDTable &methods, void *origin);  
  static NValue new_value(NValue *source, void *origin);
  static Data *send_message(Runtime &runtime, IExternalChannel &channel,
      Immediate *message);
};


class FrozenValueDTableImpl : public ValueDTable {
public:
  static ValueDTable &instance() { return instance_; }
private:
  FrozenValueDTableImpl();
  static FrozenValueDTableImpl instance_;
};


class LiveValueDTableImpl : public ValueDTable {
public:
  static ValueDTable &instance() { return instance_; }
private:
  LiveValueDTableImpl();
  static LiveValueDTableImpl instance_;
};


} // neutrino

#endif // _VALUES_CHANNEL
