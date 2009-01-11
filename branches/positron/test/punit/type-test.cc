#include "utils/string-inl.h"
#include "test-inl.h"


using namespace positron;


TEST(basic_types) {
  assert sizeof(word) == sizeof(void*);
}
