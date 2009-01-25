#include "io/miniheap.h"
#include "plankton/plankton-inl.h"
#include "plankton/codec-inl.h"
#include "utils/vector-inl.h"
#include "test-inl.h"

using namespace neutrino;

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
    Factory factory;
    p::Integer val = factory.new_integer(i);
    assert val.type() == p::Value::vtInteger;
    assert val.value() == i;
  }
}

TEST(string) {
  static const word kTestStringCount = 3;
  static const char *kTestStrings[kTestStringCount] = {
    "foobar", "", "a b c"
  };
  for (word i = 0; i < kTestStringCount; i++) {
    Factory factory;
    string str = kTestStrings[i];
    p::String val = factory.new_string(str);
    assert val.type() == p::Value::vtString;
    assert val.length() == str.length();
    for (word j = 0; j < val.length(); j++)
      assert val[j] == static_cast<uint32_t>(str[j]);
    assert val == kTestStrings[i];
  }
}

TEST(string_end) {
  Factory factory;
  p::String str = factory.new_string("foo");
  assert str[0] == static_cast<uint32_t>('f');
  assert str[1] == static_cast<uint32_t>('o');
  assert str[2] == static_cast<uint32_t>('o');
  assert str[3] == static_cast<uint32_t>('\0');
}

TEST(string_comparison) {
  Factory factory;
  assert factory.new_string("") < "a";
  assert factory.new_string("a") < "aa";
  assert factory.new_string("aa") < "aaa";
  assert factory.new_string("a") < "b";
  assert factory.new_string("aaaaa") < "aaaab";
  assert factory.new_string("bbb") > "aaaa";
  assert factory.new_string("x") != "y";
  assert factory.new_string("") == "";
  assert factory.new_string("abc") == "abc";
}

TEST(null) {
  Factory factory;
  p::Null null = factory.get_null();
  assert null.type() == p::Value::vtNull;
}

TEST(array) {
  Factory factory;
  static const word kSize = 5;
  static const char *kElements[kSize] = {
    "foo", "bar", "baz", "quux", "bleh"
  };
  p::Array array = factory.new_array(kSize);
  assert array.type() == p::Value::vtArray;
  assert array.length() == kSize;
  for (word i = 0; i < kSize; i++)
    assert array[i].type() == p::Value::vtNull;
  for (word i = 0; i < kSize; i++)
    array.set(i, factory.new_string(kElements[i]));
  for (word i = 0; i < kSize; i++) {
    p::Value val = array[i];
    assert is<p::String>(val);
    p::String str = cast<p::String>(val);
    assert str == kElements[i];
  }
  array.set(0, array);
  assert array[0] == array;
}

TEST(c_str) {
  p::String str = "flamp";
  assert str[0] == static_cast<uint32_t>('f');
  assert str[1] == static_cast<uint32_t>('l');
  assert str[2] == static_cast<uint32_t>('a');
  assert str[3] == static_cast<uint32_t>('m');
  assert str[4] == static_cast<uint32_t>('p');
  assert str[5] == static_cast<uint32_t>('\0');
  assert str.length() == 5;
  assert str == "flamp";
  assert str > "flam";
  assert str < "flampe";
  assert str > "flamo";
  assert str < "flamq";
}
