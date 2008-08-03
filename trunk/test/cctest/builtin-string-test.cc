#include "cctest/nunit-inl.h"
#include "runtime/runtime.h"
#include "utils/string.h"
#include "values/values-inl.h"

using namespace neutrino;

TEST(builtin_string_length) {
  CHECK_EQ(0, string("").length());
  CHECK_EQ(1, string("f").length());
  CHECK_EQ(2, string("fo").length());
  CHECK_EQ(3, string("foo").length());
  CHECK_EQ(3, string("foo\0bar").length());
}

TEST(builtin_string_equality) {
  CHECK(string::equals("", ""));
  CHECK(!string::equals("", "ab"));
  CHECK(!string::equals("ab", ""));
  CHECK(string::equals("ab", "ab"));
  CHECK(!string::equals("abc", "ab"));
  CHECK(!string::equals("ab", "abc"));
}

// Tests that the extended_size computation stays within 1% of the
// golden ratio for a range of large values
TEST(grow_factor) {
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

TEST(string_buffer_basic) {
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

TEST(string_buffer_printf) {
  string_buffer buf;
  buf.printf("[$0 $1 $2]", elms("foo", "bar", "baz"));
  CHECK_EQ("[foo bar baz]", buf.to_string());
  buf.clear();
  buf.printf("[% % %]", elms("foo", "bar", "baz"));
  CHECK(buf.to_string() == "[foo bar baz]");
  buf.clear();
  buf.printf("[$2 $1 $0]", elms("foo", "bar", "baz"));
  CHECK(buf.to_string() == "[baz bar foo]");
  buf.clear();
  buf.printf("$2-$1-$0", elms(6, 7, 8));
  CHECK(buf.to_string() == "8-7-6");
  buf.clear();
  buf.printf("[%{04}]", elms(26));
  CHECK(buf.to_string() == "[0026]");
}


static string format(string form, const variant &arg) {
  string_buffer buf;
  buf.printf(form, elms(arg));
  return buf.to_string();
}


TEST(int_formats) {
  CHECK_EQ("0", format("%", 0));
  CHECK_EQ("10", format("%", 10));
  CHECK_EQ("10", format("%{01}", 10));
  CHECK_EQ("10", format("%{02}", 10));
  CHECK_EQ("010", format("%{03}", 10));
  CHECK_EQ("0010", format("%{04}", 10));
  CHECK_EQ("0000000010", format("%{010}", 10));
  CHECK_EQ("10", format("%{1}", 10));
  CHECK_EQ("10", format("%{2}", 10));
  CHECK_EQ(" 10", format("%{3}", 10));
  CHECK_EQ("  10", format("%{4}", 10));
  CHECK_EQ("        10", format("%{10}", 10));
  CHECK_EQ("10", format("%{-1}", 10));
  CHECK_EQ("10", format("%{-2}", 10));
  CHECK_EQ("10 ", format("%{-3}", 10));
  CHECK_EQ("10  ", format("%{-4}", 10));
  CHECK_EQ("10        ", format("%{-10}", 10));
  CHECK_EQ("a", format("%{x1}", 10));
  CHECK_EQ(" a", format("%{x2}", 10));
  CHECK_EQ("  a", format("%{x3}", 10));
  CHECK_EQ("   a", format("%{x4}", 10));
  CHECK_EQ("         a", format("%{x10}", 10));
  CHECK_EQ("-10", format("%", -10));
  CHECK_EQ("-10", format("%{01}", -10));
  CHECK_EQ("-10", format("%{02}", -10));
  CHECK_EQ("-10", format("%{03}", -10));
  CHECK_EQ("-010", format("%{04}", -10));
}


TEST(string_null_print) {
  CHECK_EQ(7, format("[%]", string("ab\0cd", 5)).length());
}
