#include "plankton/builder.h"
#include "plankton/plankton-inl.h"
#include "plankton/codec-inl.h"
#include "utils/vector-inl.h"
#include "test-inl.h"

using namespace positron;

class StaticOutStream {
public:
  StaticOutStream(vector<uint8_t> &data) : pos_(0), data_(data) { }
  void write(uint8_t val);
private:
  int pos_;
  vector<uint8_t> &data_;
};

void StaticOutStream::write(uint8_t val) {
  data_[pos_++] = val;
}

class StaticInStream {
public:
  StaticInStream(vector<uint8_t> &data) : pos_(0), data_(data) { }
  uint8_t read();
private:
  int pos_;
  vector<uint8_t> &data_;
};

uint8_t StaticInStream::read() {
  return data_[pos_++];
}

static void test_codec(uint32_t value) {
  embed_vector<uint8_t, Codec::kMaxEncodedInt32Size> data;
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

TEST(integer) {
  for (word i = 0; i < 100; i++) {
    MessageBuffer builder;
    p_integer val = builder.new_integer(i);
    assert val.type() == p_value::vtInteger;
    assert val.value() == i;
  }
}

TEST(string) {
  static const word kTestStringCount = 3;
  static const string kTestStrings[kTestStringCount] = {
    "foobar", "", "a b c"
  };
  for (word i = 0; i < kTestStringCount; i++) {
    MessageBuffer builder;
    string str = kTestStrings[i];
    p_string val = builder.new_string(str);
    assert val.type() == p_value::vtString;
    assert val.length() == str.length();
    for (word j = 0; j < val.length(); j++)
      assert val[j] == static_cast<uint32_t>(str[j]);
    assert val == kTestStrings[i];
  }
}

TEST(string_comparison) {
  MessageBuffer builder;
  assert builder.new_string("") < string("a");
  assert builder.new_string("a") < string("aa");
  assert builder.new_string("aa") < string("aaa");
  assert builder.new_string("a") < string("b");
  assert builder.new_string("aaaaa") < string("aaaab");
  assert builder.new_string("bbb") < string("aaaa");
  assert builder.new_string("x") != string("y");
  assert builder.new_string("") == string("");
  assert builder.new_string("abc") == string("abc");
}

TEST(null) {
  MessageBuffer builder;
  p_null null = builder.get_null();
  assert null.type() == p_value::vtNull;
}

TEST(array) {
  MessageBuffer builder;
  static const word kSize = 5;
  static const string kElements[kSize] = {
    "foo", "bar", "baz", "quux", "bleh"
  };
  p_array array = builder.new_array(kSize);
  assert array.type() == p_value::vtArray;
  assert array.length() == kSize;
  for (word i = 0; i < kSize; i++)
    assert array[i].type() == p_value::vtNull;
  for (word i = 0; i < kSize; i++)
    array.set(i, builder.new_string(kElements[i]));
  for (word i = 0; i < kSize; i++) {
    p_value val = array[i];
    assert is<p_string>(val);
    p_string str = cast<p_string>(val);
    assert str == kElements[i];
  }
  array.set(0, array);
  assert array[0] == array;
}
