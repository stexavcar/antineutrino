#include "utils/string-inl.h"
#include "test-inl.h"


using namespace neutrino;


TEST(basic_types) {
  assert sizeof(word) == sizeof(void*);
  assert sizeof(uword) == sizeof(word);
  assert sizeof(uint8_t) == 1u;
  assert sizeof(int8_t) == 1u;
  assert sizeof(int32_t) == 4u;
  assert sizeof(uint32_t) == 4u;
}
