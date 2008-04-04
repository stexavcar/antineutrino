#include "cctest/tests-inl.h"
#include "utils/vector-inl.h"

using namespace neutrino;

void Test::vector() {
  int int_array[4] = { 1, 2, 3, 4 };
  // The vector is made one shorter than the array so that we can
  // safely read one element past the "last" element.
  neutrino::vector<int> ints = NEW_VECTOR(int, int_array, 3);
  CHECK_EQ(1, ints[0]);
  CHECK_EQ(2, ints[1]);
  CHECK_EQ(3, ints[2]);
  // Check that there is no bounds check in non-debug mode
  IF_NOT_DEBUG(CHECK_EQ(4, ints[3]));
  // Check that there is a bounds check in debug mode
  IF_DEBUG(CHECK_ABORTS(cnOutOfBounds, ints[3]));
}
