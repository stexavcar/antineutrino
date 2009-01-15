#include "plankton/builder.h"
#include "plankton/codec-inl.h"
#include "utils/array-inl.h"
#include "test-inl.h"

using namespace positron;

class StaticOutStream {
public:
  StaticOutStream(array<uint8_t> &data) : pos_(0), data_(data) { }
  void write(uint8_t val);
private:
  int pos_;
  array<uint8_t> &data_;
};

void StaticOutStream::write(uint8_t val) {
  data_[pos_++] = val;
}

class StaticInStream {
public:
  StaticInStream(array<uint8_t> &data) : pos_(0), data_(data) { }
  uint8_t read();
private:
  int pos_;
  array<uint8_t> &data_;
};

uint8_t StaticInStream::read() {
  return data_[pos_++];
}

static void test_codec(uint32_t value) {
  embed_array<uint8_t, Codec::kMaxEncodedInt32Size> data;
  StaticOutStream out(data);
  Encoder<StaticOutStream>::encode_uint32(value, out);
  StaticInStream in(data);
  uint32_t output = Decoder<StaticInStream>::decode_uint32(in);
  assert output == value;
}

TEST(encode) {
  for (uint32_t i = 0; i <= 1024; i++) {
    test_codec(i);
    test_codec(static_cast<uint32_t>(-i));
  }
  PseudoRandom random(43);
  for (word i = 0; i < 10000; i++)
    test_codec(static_cast<uint32_t>(random.next()));
}

TEST(builder) {
  for (int i = 0; i < 100; i++) {
    Builder builder;
    plankton::Integer val = builder.new_integer(5);
    printf("%i %i\n", val.type(), val.value());
  }
}
