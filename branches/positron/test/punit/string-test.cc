#include "utils/string-inl.h"
#include "test-inl.h"


using namespace positron;


static void try_format(string format, const var_args &args,
    string expected) {
  string_stream stream;
  stream.add(format, args);
  string result = stream.raw_c_str();
  assert result == expected;
}


TEST(simple_formatting) {
  try_format("%", args(1), "1");
  try_format("% %", args(1, 2), "1 2");
  try_format("% % %", args(1, 2, 3), "1 2 3");
}


TEST(positional) {
  try_format("@2 @1 @0", args(6, 7, 8), "8 7 6");
  try_format("@0 @1 @2", args(6, 7, 8), "6 7 8");
  try_format("@2 @2 @2", args(6, 7, 8), "8 8 8");
  try_format("@2 @1 @0 @1 @2", args(6, 7, 8), "8 7 6 7 8");
}


TEST(types) {
  try_format("--%--", args(true), "--true--");
  try_format("--%--", args(false), "--false--");
}
