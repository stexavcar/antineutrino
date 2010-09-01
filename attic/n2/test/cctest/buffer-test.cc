#include "cctest/nunit-inl.h"
#include "runtime/runtime-inl.h"
#include "utils/nonlocal.h"
#include "values/values-inl.pp.h"

using namespace neutrino;

TEST(memory) {
  LocalRuntime runtime;
  int kSize = 256;
  Buffer *buffer = runtime.heap().new_buffer<uint8_t>(kSize).value();
  @check buffer->size<uint8_t>() == kSize;
  @check buffer->size<uint16_t>() == kSize / sizeof(uint16_t);
  @check buffer->size<uword>() == kSize / sizeof(uword);
  @check buffer->size<double>() == kSize / sizeof(double);
  for (int i = 0; i < kSize; i++)
    buffer->at<uint8_t>(i) = i;
  for (int i = 0; i < kSize; i++)
    @check buffer->at<uint8_t>(i) == i;
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
