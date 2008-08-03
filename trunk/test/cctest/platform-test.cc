#include "cctest/nunit-inl.h"
#include "values/values.h"

using namespace neutrino;

// This function is used instead of sizeof to ensure that the result
// is of type uword; otherwise the conversion to a variant causes
// trouble on some platforms.
template <typename T> uword size() { return sizeof(T); }

TEST(basic_data_types) {
  CHECK_EQ(4, size<uword>());
  CHECK_EQ(4, size<word>());
  CHECK_EQ(2, size<uint16_t>());
  CHECK_EQ(2, size<int16_t>());
  CHECK_EQ(1, size<uint8_t>());
  CHECK_EQ(1, size<int8_t>());
}

TEST(data_structures) {
  CHECK_LEQ(size<Stack::Status>(), size<word>());
}
