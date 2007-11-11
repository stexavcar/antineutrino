#include "heap/values-inl.h"
#include "runtime/builtins-inl.h"
#include "runtime/runtime.h"
#include "utils/checks.h"
#include "utils/globals.h"

namespace neutrino {


// -----------------------------------
// --- I n f r a s t r u c t u r e ---
// -----------------------------------

Builtin *Builtins::get(uint32_t index) {
  switch (index) {
#define MAKE_CASE(n, type, name, str) case n: return &Builtins::type##_##name;
FOR_EACH_BUILTIN_METHOD(MAKE_CASE)
#undef MAKE_CASE
    default:
      UNREACHABLE();
      return NULL;
  }
}


// -------------------
// --- S t r i n g ---
// -------------------

Value *Builtins::string_length(Arguments &args) {
  String *self = cast<String>(args.self());
  return Smi::from_int(self->length());
}


// ---------------------------------
// --- S m a l l   I n t e g e r ---
// ---------------------------------

Value *Builtins::smi_plus(Arguments &args) {
  ASSERT(args.count() == 1);
  Smi *self = cast<Smi>(args.self());
  Smi *that = cast<Smi>(args[0]);
  return Smi::from_int(self->value() + that->value());
}

Value *Builtins::smi_minus(Arguments &args) {
  ASSERT(args.count() == 1);
  Smi *self = cast<Smi>(args.self());
  Smi *that = cast<Smi>(args[0]);
  return Smi::from_int(self->value() - that->value());
}

Value *Builtins::smi_times(Arguments &args) {
  ASSERT(args.count() == 1);
  Smi *self = cast<Smi>(args.self());
  Smi *that = cast<Smi>(args[0]);
  return Smi::from_int(self->value() * that->value());
}

Value *Builtins::smi_divide(Arguments &args) {
  ASSERT(args.count() == 1);
  Smi *self = cast<Smi>(args.self());
  Smi *that = cast<Smi>(args[0]);
  return Smi::from_int(self->value() / that->value());
}

} // namespace neutrino
