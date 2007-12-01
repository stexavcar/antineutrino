#include "cctest/tests-inl.h"
#include "heap/values-inl.h"
#include "runtime/runtime-inl.h"
#include "utils/nonlocal.h"

using namespace neutrino;

void Test::memory() {
  LocalRuntime runtime;
  int kSize = 256;
  Buffer *buffer = cast<Buffer>(runtime.heap().new_buffer<uint8_t>(kSize));
  CHECK_EQ(kSize, buffer->size<uint8_t>());
  CHECK_EQ(kSize / sizeof(uint16_t), buffer->size<uint16_t>());
  CHECK_EQ(kSize / sizeof(uint32_t), buffer->size<uint32_t>());
  CHECK_EQ(kSize / sizeof(double), buffer->size<double>());
  for (int i = 0; i < kSize; i++)
    buffer->at<uint8_t>(i) = i;
  for (int i = 0; i < kSize; i++)
    ASSERT_EQ(i, buffer->at<uint8_t>(i));
}

void Test::bounds_check() {
#ifdef DEBUG
  LocalRuntime runtime;
  Buffer *buffer = cast<Buffer>(runtime.heap().new_buffer<uint8_t>(128));
  CHECK_ABORTS(OUT_OF_BOUNDS, buffer->at<uint8_t>(128));
#endif
}

void Test::bounds_check_partial() {
#ifdef DEBUG
  LocalRuntime runtime;
  Buffer *buffer = cast<Buffer>(runtime.heap().new_buffer<uint8_t>(129));
  buffer->at<uint8_t>(128);
  CHECK_ABORTS(OUT_OF_BOUNDS, buffer->at<uint32_t>(128 / sizeof(uint32_t)));
#endif
}
