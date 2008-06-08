#include "values/values.h"

using namespace neutrino;

void Test::basic_data_types() {
  CHECK_EQ(4, sizeof(uword));
  CHECK_EQ(4, sizeof(word));
  CHECK_EQ(2, sizeof(uint16_t));
  CHECK_EQ(2, sizeof(int16_t));
  CHECK_EQ(1, sizeof(uint8_t));
  CHECK_EQ(1, sizeof(int8_t));
}

void Test::data_structures() {
  CHECK_LEQ(sizeof(Stack::Status), sizeof(word));
}
