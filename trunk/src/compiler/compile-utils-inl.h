#ifndef COMPILER_COMPILE_UTILS_INL
#define COMPILER_COMPILE_UTILS_INL

#include "compiler/compile-utils.h"
#include "utils/checks.h"

namespace neutrino {

heap_list::heap_list(Factory &factory)
    : factory_(factory)
    , length_(0) {
}

Value *heap_list::get(uword index) {
  ASSERT(index < length());
  return data()->get(index);
}

ref<Value> heap_list::operator[](uword index) {
  return new_ref(get(index));
}

}

#endif // COMPILER_COMPILE_UTILS_INL
