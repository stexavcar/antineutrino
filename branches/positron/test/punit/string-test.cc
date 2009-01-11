#include "utils/string-inl.h"
#include "check.h"

#include <cstdio>

using namespace positron;

static void simple_text_formatting(string format, const var_args &args,
    string expected) {
  string_stream stream;
  stream.add(format, args);
  string result = stream.raw_c_str();
  assert result == expected;
}

int main(int argc, char *argv[]) {
  simple_text_formatting("%", args(1), "1");
  simple_text_formatting("% %", args(1, 2), "1 2");
  simple_text_formatting("% % %", args(1, 2, 3), "1 2 3");
}
