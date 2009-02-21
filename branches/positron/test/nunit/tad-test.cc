#include "value/value-inl.h"
#include "test-inl.h"

using namespace neutrino;

TEST(types) {
  IF(cc32, return;);
  for (double d = 1.0; d < 2; d *= 1.001) {
    union {
      double v;
      uint64_t b;
    } u;
    u.v = d;
    printf("%lx %.20f\n", u.b, d);
    // assert Pointer::fits_tagged_double(d);
  }
  printf("%lx\n", Pointer::kStolenDoubleBitsMask);
}
