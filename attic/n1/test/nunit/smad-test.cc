#include "value/value-inl.h"
#include "test-inl.h"
#include "utils/log.h"

using namespace neutrino;

TEST(tagged_integer) {
  assert TaggedInteger::kUpperLimit > 0;
  assert TaggedInteger::kLowerLimit < 0;
  assert !TaggedInteger::fits(TaggedInteger::kUpperLimit);
  assert TaggedInteger::fits(TaggedInteger::kUpperLimit - 1);
  assert !TaggedInteger::fits(TaggedInteger::kLowerLimit);
  assert TaggedInteger::fits(TaggedInteger::kLowerLimit + 1);
  for (word i = 0; 0 <= i && i < TaggedInteger::kUpperLimit; i = i < 1000 ? i + 1 : grow_value(i)) {
    for (word s = -1; s < 2; s += 2) {
      word v = s * i;
      assert TaggedInteger::fits(v);
      TaggedInteger *obj = TaggedInteger::make(v);
      assert is<TaggedInteger>(obj);
      assert v == obj->value();
    }
  }
}

#ifdef M64

class Double {
public:
  Double(double value) : value_(value) { }
  bool operator<(double other) { return value_ < other; }
  void operator*=(double other) { value_ *= other; }
  uint64_t bits();
  word raw_exponent();
  word exponent();
  double value() { return value_; }
private:
  double value_;
};

uint64_t Double::bits() {
  return double_bits(value_);
}

word Double::raw_exponent() {
  return ((bits() >> 52) & ((1 << 11) - 1));
}

word Double::exponent() {
  return raw_exponent() - 1023;
}

static word integer_log2(double value) {
  if (value < 0.0) {
    return integer_log2(-value);
  } else if (value < 1.0) {
    return -integer_log2(2.0 / value);
  } else {
    word result = 0;
    while (value > 2.0) {
      result += 1;
      value /= 2.0;
    }
    return result;
  }
}

static double pow(double x, word n) {
  if (n == 1) return x;
  else return x * pow(x, n - 1);
}

TEST(range_fits) {
  static const word kRemainingBits = 8;
  static const word kOffset = (1 << (kRemainingBits - 1));
  static const word kStolenLimit = (1 << kRemainingBits) - 1;
  static const double kLimit = pow(2.0, kStolenLimit / 2);
  for (double v = 1.0 / kLimit; v < kLimit; v *= 1.001) {
    for (word sign = 1; sign >= -1; sign -= 2) {
      Double d(sign * v);
      assert d.exponent() == integer_log2(d.value());
      word subject = d.exponent() + kOffset;
      assert 0 <= subject;
      assert subject <= kStolenLimit;
      assert Pointer::fits_small_double(d.value());
      SmallDouble *small = Pointer::tag_small_double(d.value());
      assert is<SmallDouble>(small);
      assert small->value() == d.value();
    }
  }
}

#endif
