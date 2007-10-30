#include "heap/values-inl.h"
#include "runtime/runtime.h"
#include "utils/string.h"

using namespace neutrino;

static void test_builtin_string_length() {
  CHECK_EQ(0, string("").length());
  CHECK_EQ(1, string("f").length());
  CHECK_EQ(2, string("fo").length());
  CHECK_EQ(3, string("foo").length());
  CHECK_EQ(3, string("foo\0bar").length());
}

static void test_builtin_string_equality() {
  CHECK(string::equals("", ""));
  CHECK(!string::equals("", "ab"));
  CHECK(!string::equals("ab", ""));
  CHECK(string::equals("ab", "ab"));
  CHECK(!string::equals("abc", "ab"));
  CHECK(!string::equals("ab", "abc"));
}

// Tests that the extended_size computation stays within 1% of the
// golden ratio for a range of large values
static void test_grow_factor() {
  static const float kGoldenRatio = 1.6180339887;
  static const float kDeviation = 0.01;
  static const float kLowerRatio = kGoldenRatio * (1 - kDeviation);
  static const float kUpperRatio = kGoldenRatio * (1 + kDeviation);
  for (uint32_t i = 100; i < 100000000; i = static_cast<uint32_t>(i * 1.1)) {
    uint32_t size = grow_value(i);
    uint32_t lower_limit = static_cast<uint32_t>(kLowerRatio * i);
    CHECK(lower_limit < size);
    uint32_t upper_limit = static_cast<uint32_t>(kUpperRatio * i);
    CHECK(size < upper_limit);
  }
}

static void test_string_buffer_basic() {
  string str;
  {
    string_buffer buf(1);
    str = buf.to_string();
    ASSERT(str == "");
    str.dispose();
    buf.append("foo");
    buf.append('-');
    buf.append("bar"); 
    str = buf.to_string();
    ASSERT(str == "foo-bar");
    buf.clear();
    ASSERT(str == "foo-bar");
    str.dispose();
    buf.append("nirk!");
    str = buf.to_string();
    ASSERT(str == "nirk!");
  }
  ASSERT(str == "nirk!");
  str.dispose();
}

static void test_printf() {
  string_buffer buf;
  buf.printf("[$0 $1 $2]", "foo", "bar", "baz");
  ASSERT(buf.to_string() == "[foo bar baz]");
  buf.clear();
  buf.printf("[% % %]", "foo", "bar", "baz");
  ASSERT(buf.to_string() == "[foo bar baz]");
  buf.clear();
  buf.printf("[$2 $1 $0]", "foo", "bar", "baz");
  ASSERT(buf.to_string() == "[baz bar foo]");
  buf.clear();
  buf.printf("$2-$1-$0", 6, 7, 8);
  ASSERT(buf.to_string() == "8-7-6");
  buf.clear();
  buf.printf("$0{4.}", 21);
  ASSERT(buf.to_string() == "  21");
  buf.clear();
  buf.printf("[$0{.4}]", 3.1415926);
  ASSERT(buf.to_string() == "[3.142]");
  buf.clear();
  buf.printf("[%{04}]", 26);
  ASSERT(buf.to_string() == "[0026]");
}
