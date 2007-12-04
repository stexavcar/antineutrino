using namespace neutrino;

void Test::assertions() {
  CHECK_EQ(sizeof(uint32_t), 4);
  CHECK_EQ(sizeof(int32_t), 4);
  CHECK_EQ(sizeof(uint16_t), 2);
  CHECK_EQ(sizeof(int16_t), 2);
  CHECK_EQ(sizeof(uint8_t), 1);
  CHECK_EQ(sizeof(int8_t), 1);
}
