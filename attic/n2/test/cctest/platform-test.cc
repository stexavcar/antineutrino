#include "cctest/nunit-inl.h"
#include "values/values.h"

using namespace neutrino;

// This function is used instead of sizeof to ensure that the result
// is of type uword; otherwise the conversion to a variant causes
// trouble on some platforms.
template <typename T> uword size() { return sizeof(T); }

TEST(basic_data_types) {
  @check size<uword>() == 4;
  @check size<word>() == 4;
  @check size<uint16_t>() == 2;
  @check size<int16_t>() == 2;
  @check size<uint8_t>() == 1;
  @check size<int8_t>() == 1;
}

TEST(data_structures) {
  CHECK_LEQ(size<Stack::Status>(), size<word>());
}
