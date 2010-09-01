#include "io/miniheap-inl.h"
#include "plankton/plankton-inl.h"
#include "plankton/codec-inl.h"
#include "utils/vector-inl.h"
#include "test-inl.h"

using namespace neutrino;

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
  p::String str = "foo";
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
  MutableArray array = factory.new_array(kSize);
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

TEST(empty_array) {
  p::Array empty = p::Array::empty();
  assert (is<p::Array>(empty));
  assert empty.length() == 0;
}

static void test_literal_array(p::Value val) {
  assert (is<p::Array>(val));
  p::Array that = cast<p::Array>(val);
  assert that.length() == 3;
  assert (cast<p::Integer>(that[0])).value() == 1;
  assert (cast<p::String>(that[1])) == "foo";
  p::Array inner = cast<p::Array>(that[2]);
  assert inner.length() == 2;
  assert (cast<p::String>(inner[0])) == "bar";
  assert (cast<p::Integer>(inner[1])).value() == 6;
}

TEST(literal_array) {
  test_literal_array(p::Array::of(1, "foo", p::Array::of("bar", 6)));
}

class TestObjectProxy {
public:
  TestObjectProxy();
  p::Object as_object() { return dtable().proxy_for(*this); }
  p::Value inc(Message &message);
  p::Value dec(Message &message);
  p::Value set(Message &message);
  word value() { return value_; }
private:
  word value_;
  ObjectProxyDTable<TestObjectProxy> &dtable() { return dtable_; }
  ObjectProxyDTable<TestObjectProxy> dtable_;
};

TestObjectProxy::TestObjectProxy() : value_(0) {
  dtable().add_method("inc", &TestObjectProxy::inc);
  dtable().add_method("dec", &TestObjectProxy::dec);
  dtable().add_method("set", &TestObjectProxy::set);
}

p::Value TestObjectProxy::inc(Message &message) {
  assert message.args().length() == 0;
  value_++;
  return Factory::get_null();
}

p::Value TestObjectProxy::dec(Message &message) {
  assert message.args().length() == 0;
  value_--;
  return Factory::get_null();
}

p::Value TestObjectProxy::set(Message &message) {
  assert message.args().length() == 1;
  assert (is<p::Integer>(message.args()[0]));
  assert cast<p::Integer>(message.args()[0]).value() == 27;
  return Factory::get_null();
}

TEST(proxy) {
  TestObjectProxy proxy;
  p::Object obj = proxy.as_object();
  assert proxy.value() == 0;
  assert (is<p::Null>(obj.send("inc")));
  assert proxy.value() == 1;
  for (word i = 0; i < 10; i++)
    obj.send("inc");
  assert proxy.value() == 11;
  for (word i = 0; i < 5; i++)
    obj.send("dec");
  assert proxy.value() == 6;
  obj.send("set", p::Array::of(27));
}
