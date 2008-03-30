#include "runtime/runtime.h"
#include "utils/string.h"
#include "values/values-inl.h"

using namespace neutrino;

void Test::builtin_string_length() {
  CHECK_EQ(0, string("").length());
  CHECK_EQ(1, string("f").length());
  CHECK_EQ(2, string("fo").length());
  CHECK_EQ(3, string("foo").length());
  CHECK_EQ(3, string("foo\0bar").length());
}

void Test::builtin_string_equality() {
  CHECK(string::equals("", ""));
  CHECK(!string::equals("", "ab"));
  CHECK(!string::equals("ab", ""));
  CHECK(string::equals("ab", "ab"));
  CHECK(!string::equals("abc", "ab"));
  CHECK(!string::equals("ab", "abc"));
}

// Tests that the extended_size computation stays within 1% of the
// golden ratio for a range of large values
void Test::grow_factor() {
  static const double kGoldenRatio = 1.6180339887;
  static const double kDeviation = 0.01;
  static const double kLowerRatio = kGoldenRatio * (1 - kDeviation);
  static const double kUpperRatio = kGoldenRatio * (1 + kDeviation);
  for (uword i = 100; i < 100000000; i = static_cast<uword>(i * 1.1)) {
    uword size = grow_value(i);
    uword lower_limit = static_cast<uword>(kLowerRatio * i);
    CHECK(lower_limit < size);
    uword upper_limit = static_cast<uword>(kUpperRatio * i);
    CHECK(size < upper_limit);
  }
}

void Test::string_buffer_basic() {
  string str;
  {
    string_buffer buf(1);
    str = buf.to_string();
    CHECK(str == "");
    str.dispose();
    buf.append("foo");
    buf.append('-');
    buf.append("bar"); 
    str = buf.to_string();
    CHECK(str == "foo-bar");
    buf.clear();
    CHECK(str == "foo-bar");
    str.dispose();
    buf.append("nirk!");
    str = buf.to_string();
    CHECK(str == "nirk!");
  }
  CHECK(str == "nirk!");
  str.dispose();
}

void Test::printf() {
  string_buffer buf;
  buf.printf("[$0 $1 $2]", "foo", "bar", "baz");
  CHECK(buf.to_string() == "[foo bar baz]");
  buf.clear();
  buf.printf("[% % %]", "foo", "bar", "baz");
  CHECK(buf.to_string() == "[foo bar baz]");
  buf.clear();
  buf.printf("[$2 $1 $0]", "foo", "bar", "baz");
  CHECK(buf.to_string() == "[baz bar foo]");
  buf.clear();
  buf.printf("$2-$1-$0", 6, 7, 8);
  CHECK(buf.to_string() == "8-7-6");
  buf.clear();
  buf.printf("$0{4.}", 21);
  CHECK(buf.to_string() == "  21");
  buf.clear();
  buf.printf("[$0{.4}]", 3.1415926);
  CHECK(buf.to_string() == "[3.142]");
  buf.clear();
  buf.printf("[%{04}]", 26);
  CHECK(buf.to_string() == "[0026]");
}
