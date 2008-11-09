#include "cctest/nunit-inl.h"
#include "utils/array-inl.h"

using namespace neutrino;

TEST(array) {
  int int_array[4] = { 1, 2, 3, 4 };
  // The array is made one shorter than the array so that we can
  // safely read one element past the "last" element.
  neutrino::array<int> ints = NEW_ARRAY(int_array, 3);
  @check ints[0] == 1;
  @check ints[1] == 2;
  @check ints[2] == 3;
  // Check that there is no bounds check in non-debug mode
  IF_NOT_DEBUG(@check ints[3] == 4);
  // Check that there is a bounds check in debug mode
  IF_DEBUG(CHECK_ABORTS(cnOutOfBounds, ints[3]));
}
