#include "cctest/nunit-inl.h"
#include "utils/smart-ptrs-inl.h"

using namespace neutrino;

static const uword kSize = 3;
// Make the backing array one element larger than the bounds to
// allow us to read past the end in non-paranoid mode
static word raw_ints[kSize + 1] = {9, 8, 7, 6};

static bounded_ptr<word> ints_start() {
  return NEW_BOUNDED_PTR(raw_ints, raw_ints, raw_ints + kSize);
}


TEST(simple_bounded_ptr_test) {
  bounded_ptr<int> ints = ints_start();
  @check ints[0] == 9;
  @check ints[1] == 8;
  @check ints[2] == 7;
  // Paranoid mode does bounds checking
  IF_PARANOID(CHECK_ABORTS(cnOutOfBounds, ints[3]));
  // Non-paranoid mode doesn't
  IF_NOT_PARANOID(@check ints[3] == 6);
}


TEST(bounded_ptr_simple_arith_test) {
  bounded_ptr<int> ints = ints_start();
  bounded_ptr<int> second = ints + 2;
  @check second[0] == 7;
  IF_PARANOID(CHECK_ABORTS(cnOutOfBounds, second[1]));
  IF_NOT_PARANOID(@check second[1] == 6);
}
