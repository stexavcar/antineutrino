#include "heap/zone-inl.h"
#include "io/sexp.h"
#include "runtime/runtime.h"

namespace n = neutrino;

using namespace neutrino::sexp;

static Sexp *parse(n::Runtime &runtime, const char *str) {
  n::RefScope scope;
  n::ref<n::String> obj = runtime.factory().new_string(str);
  SymbolResolver resolver;
  return Reader::read(obj, resolver);
}

static void test_parsing() {
  n::Runtime runtime;
  runtime.initialize();
  n::zone::Zone zone;
  CHECK(parse(runtime, "'abcdefg'")->equals(*new String("abcdefg")));
  CHECK(parse(runtime, "100")->equals(*new Number(100)));
  CHECK(parse(runtime, "('a' 'b' 'c')")->equals(*new List(*new String("a"), *new String("b"), *new String("c"))));
}

static void test_equality() {
  n::zone::Zone zone;
  CHECK((new String("abc"))->equals(*new String("abc")));
  CHECK(!(new String("ab"))->equals(*new String("abc")));
  CHECK(!(new String("abc"))->equals(*new String("bc")));
  CHECK((new Number(10))->equals(*new Number(10)));
  CHECK(!(new Number(9))->equals(*new Number(10)));
  CHECK(!(new Number(10))->equals(*new String("ab")));
  CHECK(!(new String("10"))->equals(*new Number(6)));
  CHECK((new List(*new String("a"), *new String("b")))->equals(*new List(*new String("a"), *new String("b"))));
  CHECK(!(new List(*new String("a"), *new String("c")))->equals(*new List(*new String("a"), *new String("b"))));
}
