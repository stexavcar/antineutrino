#include "heap/values-inl.h"
#include "runtime/builtins-inl.h"
#include "runtime/runtime.h"
#include "utils/checks.h"
#include "utils/globals.h"

namespace neutrino {


// -----------------------------------
// --- I n f r a s t r u c t u r e ---
// -----------------------------------

enum BuiltinTag {
  __first_builtin_tag = -1
#define DECLARE_TAG(n, Class, name) , Class##_##name = n
FOR_EACH_BUILTIN(DECLARE_TAG)
#undef DECLARE_TAG
};

MAKE_ENUM_INFO_HEADER(BuiltinTag)
#define MAKE_ENTRY(n, Class, name) MAKE_ENUM_INFO_ENTRY(Class##_##name)
FOR_EACH_BUILTIN(MAKE_ENTRY)
#undef MAKE_ENTRY
MAKE_ENUM_INFO_FOOTER()

Builtin *Builtins::get(uint32_t index) {
  switch (index) {
#define MAKE_CASE(n, Class, name) case n: return &Builtins::Class##_##name;
FOR_EACH_BUILTIN(MAKE_CASE)
#undef MAKE_CASE
    default:
      UNHANDLED(BuiltinTag, index);
      return NULL;
  }
}


// -------------------
// --- S t r i n g ---
// -------------------

Value *Builtins::String_length(Arguments &args) {
  String *self = cast<String>(args.self());
  return Smi::from_int(self->length());
}


// ---------------------------------
// --- S m a l l   I n t e g e r ---
// ---------------------------------

Value *Builtins::Smi_plus(Arguments &args) {
  ASSERT(args.count() == 1);
  Smi *self = cast<Smi>(args.self());
  Smi *that = cast<Smi>(args[0]);
  return Smi::from_int(self->value() + that->value());
}

Value *Builtins::Smi_minus(Arguments &args) {
  ASSERT(args.count() == 1);
  Smi *self = cast<Smi>(args.self());
  Smi *that = cast<Smi>(args[0]);
  return Smi::from_int(self->value() - that->value());
}

} // namespace neutrino
