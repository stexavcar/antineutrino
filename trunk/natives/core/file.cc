#include <stdio.h>

#include "runtime/builtins-inl.h"
#include "runtime/runtime.h"
#include "utils/scoped-ptrs.h"
#include "values/values-inl.h"

namespace neutrino {

extern "C" Data *native_open_file(BuiltinArguments &args) {
  ASSERT_EQ(1, args.count());
  String *name_obj = cast<String>(to<String>(args[0]));
  own_vector<char> name(name_obj->c_str());
  FILE *file = fopen(name.data(), "r");
  if (file == NULL) return args.runtime().roots().nuhll();
  return Smi::from_int(reinterpret_cast<uword>(file));
}

extern "C" Data *native_close_file(BuiltinArguments &args) {
  ASSERT_EQ(1, args.count());
  Smi *handle_obj = cast<Smi>(to<Smi>(args[0]));
  FILE *file = reinterpret_cast<FILE*>(handle_obj->value());
  fclose(file);
  return args.runtime().roots().vhoid();
}

extern "C" Data *native_read_file(BuiltinArguments &args) {
  ASSERT_EQ(1, args.count());
  Smi *handle_obj = cast<Smi>(to<Smi>(args[0]));
  FILE *file = reinterpret_cast<FILE*>(handle_obj->value());
  fseek(file, 0, SEEK_END);
  uword size = ftell(file);
  rewind(file);
  String *result = cast<String>(args.runtime().heap().new_string(size));
  uword offset = 0;
  while (offset < size) {
    const uword kSize = 256;
    uint8_t bytes[kSize];
    uword count = fread(bytes, 1, kSize, file);
    for (uword i = 0; i < count; i++, offset++)
      result->set(offset, bytes[i]);
  }
  return result;
}

} // neutrino
