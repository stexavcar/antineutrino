#include "heap/pointer-inl.h"
#include "heap/space.h"
#include "utils/checks.h"

using namespace neutrino;

address Space::allocate_raw(uint32_t size) {
  ASSERT(ValuePointer::is_aligned(size));
  return new int8_t[size];
}
