#ifndef _HEAP_FACTORY
#define _HEAP_FACTORY

#include "compiler/ast.h"
#include "heap/values.h"
#include "utils/string.h"
#include "utils/globals.h"

namespace neutrino {

class Factory {
public:
  Factory(Runtime &heap);
  ref<String> new_string(string str);
  ref<Tuple> new_tuple(uint32_t size);
  ref<Lambda> new_lambda(uint32_t argc, ref<Code> code, ref<Tuple> literals);
  ref<Method> new_method(ref<String> name, ref<Lambda> lambda);
  ref<Dictionary> new_dictionary();
  ref<Code> new_code(uint32_t size);
private:
  Runtime &runtime() { return runtime_; }
  Runtime &runtime_;
};

}

#endif // _HEAP_FACTORY
