#include "values/values.h"

using namespace neutrino;

void Test::basic_data_types() {
  CHECK_EQ(4, sizeof(uint32_t));
  CHECK_EQ(4, sizeof(int32_t));
  CHECK_EQ(2, sizeof(uint16_t));
  CHECK_EQ(2, sizeof(int16_t));
  CHECK_EQ(1, sizeof(uint8_t));
  CHECK_EQ(1, sizeof(int8_t));
}

void Test::data_structures() {
  CHECK_LT(sizeof(Stack::Status), 4);
}
