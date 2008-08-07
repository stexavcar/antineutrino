#include "cctest/nunit-inl.h"
#include "runtime/runtime-inl.h"
#include "utils/nonlocal.h"
#include "values/values-inl.h"

using namespace neutrino;

TEST(memory) {
  LocalRuntime runtime;
  int kSize = 256;
  Buffer *buffer = runtime.heap().new_buffer<uint8_t>(kSize).value();
  CHECK_EQ(kSize, buffer->size<uint8_t>());
  CHECK_EQ(kSize / sizeof(uint16_t), buffer->size<uint16_t>());
  CHECK_EQ(kSize / sizeof(uword), buffer->size<uword>());
  CHECK_EQ(kSize / sizeof(double), buffer->size<double>());
  for (int i = 0; i < kSize; i++)
    buffer->at<uint8_t>(i) = i;
  for (int i = 0; i < kSize; i++)
    ASSERT_EQ(i, buffer->at<uint8_t>(i));
}

TEST(bounds_check) {
#ifdef DEBUG
  LocalRuntime runtime;
  Buffer *buffer = runtime.heap().new_buffer<uint8_t>(128).value();
  CHECK_ABORTS(cnOutOfBounds, buffer->at<uint8_t>(128));
#endif
}

TEST(bounds_check_partial) {
#ifdef DEBUG
  LocalRuntime runtime;
  Buffer *buffer = runtime.heap().new_buffer<uint8_t>(129).value();
  buffer->at<uint8_t>(128);
  CHECK_ABORTS(cnOutOfBounds, buffer->at<uword>(128 / sizeof(uword)));
#endif
}
