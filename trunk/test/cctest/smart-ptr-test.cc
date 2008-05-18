#include "cctest/tests-inl.h"
#include "utils/smart-ptrs-inl.h"

using namespace neutrino;

static const uword kSize = 3;
// Make the backing array one element larger than the bounds to
// allow us to read past the end in non-paranoid mode
static word raw_ints[kSize + 1] = {9, 8, 7, 6};

static checked_ptr<word> ints_start() {
  return NEW_CHECKED_PTR(raw_ints, raw_ints, raw_ints + kSize);
}


void Test::simple_checked_ptr_test() {
  checked_ptr<int> ints = ints_start();
  CHECK_EQ(9, ints[0]);
  CHECK_EQ(8, ints[1]);
  CHECK_EQ(7, ints[2]);
  // Paranoid mode does bounds checking
  IF_PARANOID(CHECK_ABORTS(cnOutOfBounds, ints[3]));
  // Non-paranoid mode doesn't
  IF_NOT_PARANOID(CHECK_EQ(6, ints[3]));
}


void Test::checked_ptr_simple_arith_test() {
  checked_ptr<int> ints = ints_start();
  checked_ptr<int> second = ints + 2;
  CHECK_EQ(7, second[0]);
  IF_PARANOID(CHECK_ABORTS(cnOutOfBounds, second[1]));
  IF_NOT_PARANOID(CHECK_EQ(6, second[1]));
}
