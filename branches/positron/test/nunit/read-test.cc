#include "io/read-inl.h"
#include "utils/arena-inl.h"
#include "test-inl.h"

using namespace neutrino;

static bool operator==(vector<uint8_t> vect, string str) {
  if (vect.length() != str.length())
    return false;
  for (word i = 0; i < vect.length(); i++) {
    if (vect[i] != str[i])
      return false;
  }
  return true;
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
  SexpScanner scan("foo.bar foo-bar foo=bar", arena);
  assert scan.next() == SexpScanner::tString;
  assert scan.last_string() == "foo.bar";
  assert scan.next() == SexpScanner::tString;
  assert scan.last_string() == "foo-bar";
  assert scan.next() == SexpScanner::tString;
  assert scan.last_string() == "foo=bar";
}

TEST(parser) {
  {
    Arena arena;
    s_exp *expr = s_exp::read("foo", arena);
    assert is<s_string>(expr);
    assert cast<s_string>(expr)->chars() == "foo";
  }
  {
    Arena arena;
    s_exp *expr = s_exp::read("(foo bar baz)", arena);
    assert is<s_list>(expr);
    s_list *list = cast<s_list>(expr);
    assert list->length() == 3;
    assert cast<s_string>(list->get(0))->chars() == "foo";
    assert cast<s_string>(list->get(1))->chars() == "bar";
    assert cast<s_string>(list->get(2))->chars() == "baz";
  }
  {
    Arena arena;
    s_exp *expr = s_exp::read("(foo (bar baz) quux)", arena);
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

TEST(match) {
  {
    Arena arena;
    s_exp *expr = s_exp::read("(foo bar baz)", arena);
    assert (expr->match(m_list(m_string("foo"), m_string("bar"), m_string("baz"))));
  }
  {
    Arena arena;
    s_exp *expr = s_exp::read("(dim dam dum)", arena);
    vector<uint8_t> middle;
    assert (expr->match(m_list(m_string("dim"), m_string(&middle), m_string("dum"))));
    assert middle == "dam";
  }
  {
    Arena arena;
    s_exp *expr = s_exp::read("((a b) c (d (e f)))", arena);
    vector<uint8_t> selected;
    bool match = expr->match(
        m_list(
            m_list(
                m_string("a"),
                m_string("b")),
            m_string("c"),
            m_list(
                m_string("d"),
                m_list(
                    m_string(&selected),
                    m_string("f")))));
    assert match;
    assert selected == "e";
  }
}