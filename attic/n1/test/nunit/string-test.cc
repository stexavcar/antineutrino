#include "utils/string-inl.h"
#include "test-inl.h"


using namespace neutrino;


static void try_format(string format, const var_args &args,
    string expected) {
  string_stream stream;
  stream.add(format, args);
  string result = stream.raw_string();
  assert result == expected;
}


TEST(simple_formatting) {
  try_format("%", vargs(1), "1");
  try_format("% %", vargs(1, 2), "1 2");
  try_format("% % %", vargs(1, 2, 3), "1 2 3");
}


TEST(positional) {
  try_format("@2 @1 @0", vargs(6, 7, 8), "8 7 6");
  try_format("@0 @1 @2", vargs(6, 7, 8), "6 7 8");
  try_format("@2 @2 @2", vargs(6, 7, 8), "8 8 8");
  try_format("@2 @1 @0 @1 @2", vargs(6, 7, 8), "8 7 6 7 8");
}


TEST(types) {
  try_format("--%--", vargs(true), "--true--");
  try_format("--%--", vargs(false), "--false--");
}


TEST(hex) {
  try_format("--%{x}--", vargs(10), "--a--");
  try_format("--%{8x}--", vargs(10), "--       a--");
  try_format("--%{08x}--", vargs(10), "--0000000a--");
}