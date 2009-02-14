#include "io/read-inl.h"
#include "utils/arena-inl.h"
#include "test-inl.h"

using namespace neutrino;

static s_exp *read(string input, Arena &arena) {
  SexpParser parser(input, arena);
  return parser.read();
}

TEST(trivial_scan) {
  Arena arena;
  SexpScanner scan(" ( foo ) ", arena);
  assert scan.last_string().is_empty();
  assert scan.next() == SexpScanner::tLeft;
  assert scan.last_string().is_empty();
  assert scan.next() == SexpScanner::tString;
  assert scan.last_string() == "foo";
  assert scan.next() == SexpScanner::tRight;
  assert scan.last_string().is_empty();
  assert scan.next() == SexpScanner::tEnd;
  assert scan.last_string().is_empty();
}

TEST(tokens) {
  Arena arena;
  SexpScanner scan("foo.bar foo-bar 4 foo=bar", arena);
  assert scan.next() == SexpScanner::tString;
  assert scan.last_string() == "foo.bar";
  assert scan.next() == SexpScanner::tString;
  assert scan.last_string() == "foo-bar";
  assert scan.next() == SexpScanner::tNumber;
  assert scan.last_number() == 4;
  assert scan.next() == SexpScanner::tString;
  assert scan.last_string() == "foo=bar";
}

TEST(parser) {
  {
    Arena arena;
    s_exp *expr = read("foo", arena);
    assert is<s_string>(expr);
    assert cast<s_string>(expr)->chars() == "foo";
  }
  {
    Arena arena;
    s_exp *expr = read("(foo bar baz)", arena);
    assert is<s_list>(expr);
    s_list *list = cast<s_list>(expr);
    assert list->length() == 3;
    assert cast<s_string>(list->get(0))->chars() == "foo";
    assert cast<s_string>(list->get(1))->chars() == "bar";
    assert cast<s_string>(list->get(2))->chars() == "baz";
  }
  {
    Arena arena;
    s_exp *expr = read("(foo (bar baz) quux)", arena);
    assert is<s_list>(expr);
    s_list *list = cast<s_list>(expr);
    assert list->length() == 3;
    assert cast<s_string>(list->get(0))->chars() == "foo";
    s_list *inner = cast<s_list>(list->get(1));
    assert inner->length() == 2;
    assert cast<s_string>(inner->get(0))->chars() == "bar";
    assert cast<s_string>(inner->get(1))->chars() == "baz";
    assert cast<s_string>(list->get(2))->chars() == "quux";
  }
}
