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
  ASSERT(parse(runtime, "'abcdefg'")->equals(*new String("abcdefg")));
  ASSERT(parse(runtime, "100")->equals(*new Number(100)));
  ASSERT(parse(runtime, "('a' 'b' 'c')")->equals(*new List(*new String("a"), *new String("b"), *new String("c"))));
}

static void test_equality() {
  n::zone::Zone zone;
  ASSERT((new String("abc"))->equals(*new String("abc")));
  ASSERT(!(new String("ab"))->equals(*new String("abc")));
  ASSERT(!(new String("abc"))->equals(*new String("bc")));
  ASSERT((new Number(10))->equals(*new Number(10)));
  ASSERT(!(new Number(9))->equals(*new Number(10)));
  ASSERT(!(new Number(10))->equals(*new String("ab")));
  ASSERT(!(new String("10"))->equals(*new Number(6)));
  ASSERT((new List(*new String("a"), *new String("b")))->equals(*new List(*new String("a"), *new String("b"))));
  ASSERT(!(new List(*new String("a"), *new String("c")))->equals(*new List(*new String("a"), *new String("b"))));
}
