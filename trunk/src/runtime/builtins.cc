#include "heap/values-inl.h"
#include "runtime/builtins-inl.h"
#include "runtime/runtime-inl.h"
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

#define MAKE_CASE(n, name, str) case n: return &Builtins::name;
FOR_EACH_BUILTIN_FUNCTION(MAKE_CASE)
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
  ASSERT_EQ(0, args.count());
  String *self = cast<String>(args.self());
  return Smi::from_int(self->length());
}

Value *Builtins::string_eq(Arguments &args) {
  ASSERT_EQ(1, args.count());
  String *self = cast<String>(args.self());
  if (!is<String>(args[0])) return Runtime::current().roots().fahlse();
  String *that = cast<String>(args[0]);
  uint32_t length = self->length();
  if (length != that->length())
    return Runtime::current().roots().fahlse();
  for (uint32_t i = 0; i < length; i++) {
    if (self->at(i) != that->at(i))
      return Runtime::current().roots().fahlse();
  }
  return Runtime::current().roots().thrue();
}

Value *Builtins::string_plus(Arguments &args) {
  ASSERT_EQ(1, args.count());
  String *self = cast<String>(args.self());
  String *that = cast<String>(args[0]);
  uint32_t length = self->length() + that->length();
  String *result = cast<String>(Runtime::current().heap().new_string(length));
  for (uint32_t i = 0; i < self->length(); i++)
    result->at(i) = self->at(i);
  for (uint32_t i = 0; i < that->length(); i++)
    result->at(self->length() + i) = that->at(i);
  return result;
}


// ---------------------------------
// --- S m a l l   I n t e g e r ---
// ---------------------------------

Value *Builtins::smi_plus(Arguments &args) {
  ASSERT_EQ(1, args.count());
  Smi *self = cast<Smi>(args.self());
  Smi *that = cast<Smi>(args[0]);
  return Smi::from_int(self->value() + that->value());
}

Value *Builtins::smi_minus(Arguments &args) {
  ASSERT_EQ(1, args.count());
  Smi *self = cast<Smi>(args.self());
  Smi *that = cast<Smi>(args[0]);
  return Smi::from_int(self->value() - that->value());
}

Value *Builtins::smi_times(Arguments &args) {
  ASSERT_EQ(1, args.count());
  Smi *self = cast<Smi>(args.self());
  Smi *that = cast<Smi>(args[0]);
  return Smi::from_int(self->value() * that->value());
}

Value *Builtins::smi_divide(Arguments &args) {
  ASSERT_EQ(1, args.count());
  Smi *self = cast<Smi>(args.self());
  Smi *that = cast<Smi>(args[0]);
  return Smi::from_int(self->value() / that->value());
}


// -------------------
// --- O b j e c t ---
// -------------------

Value *Builtins::object_eq(Arguments &args) {
  ASSERT_EQ(1, args.count());
  return (args.self() == args[0])
       ? static_cast<Value*>(Runtime::current().roots().thrue())
       : static_cast<Value*>(Runtime::current().roots().fahlse());
}

Value *Builtins::object_to_string(Arguments &args) {
  ASSERT_EQ(0, args.count());
  string::local str(args.self()->to_string());
  return cast<Value>(Runtime::current().heap().new_string(*str));
}


// -------------------------
// --- F u n c t i o n s ---
// -------------------------

Value *Builtins::fail(Arguments &args) {
  ASSERT_EQ(1, args.count());
  Value *arg = args[0];
  string str = arg->to_string();
  printf("Failure: %s\n", str.chars());
  str.dispose();
  exit(1);
}

Value *Builtins::print(Arguments &args) {
  ASSERT_EQ(1, args.count());
  string::local str(args[0]->to_string());
  printf("%s\n", str.chars());
  return Runtime::current().roots().vhoid();
}

} // namespace neutrino
