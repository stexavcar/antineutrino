#include "plankton/codec-inl.h"
#include "plankton/match.h"
#include "plankton/plankton-inl.h"
#include "utils/smart-ptrs-inl.h"
#include "test-inl.h"

using namespace neutrino;

TEST(integer_match) {
  p::Value value = 1;
  assert value.match(1);
  assert !value.match(-1);
  assert !value.match(static_cast<word>(0));
  word out = 0;
  assert value.match(&out);
  assert out == 1;
}

TEST(string_match) {
  p::Value value = "nork";
  assert value.match("nork");
  assert !value.match("");
  assert !value.match("offel");
  own_vector<char> out;
  assert value.match(out);
  assert string("nork") == out.start();
}

#define ao p::Array::of
#define po p::Pattern::of

TEST(array_match) {
  word middle = 0;
  assert ao(1).match(po(1));
  assert !ao(1).match(po(2));
  assert ao(1, 2).match(po(1, 2));
  assert !ao(1, 2).match(po(1, 1));
  assert ao(1, 2, 3).match(po(1, 2, 3));
  assert !ao(1, 2, 3).match(po(1, 2, 1));
  assert ao(1, 2, 3, 4).match(po(1, 2, 3, 4));
  assert !ao(1, 2, 3, 4).match(po(1, 2, 3, 1));
  assert ao(1, 2, 3, 4, 5).match(po(1, 2, 3, 4, 5));
  assert !ao(1, 2, 3, 4, 5).match(po(1, 2, 3, 4, 1));
  assert ao(1, 2, 3, 4, 5, 6).match(po(1, 2, 3, 4, 5, 6));
  assert !ao(1, 2, 3, 4, 5).match(po(1, 2, 3, 4, 5, 1));
  assert ao(1, 2, 3).match(po(1, &middle, 3));
  assert middle == 2;
  assert !ao(1, 2, 3).match(po(&middle, &middle));
  assert !ao(1, 2, 3).match(po(&middle, &middle, &middle, &middle));
}
