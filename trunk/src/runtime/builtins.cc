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
  Value *self = args.self();
  return self;
}

} // namespace neutrino
