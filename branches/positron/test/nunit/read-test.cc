#include "io/read.h"
#include "io/write.h"
#include "plankton/plankton-inl.h"
#include "utils/arena-inl.h"
#include "test-inl.h"

using namespace neutrino;

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
  p::Object reader = p::ServiceRegistry::lookup("neutrino.io.read");
  assert !reader.is_empty();
  {
    p::MessageData data;
    p::Value expr = reader.send("parse", p::Array::of("foo"), &data);
    assert is<p::String>(expr);
    assert cast<p::String>(expr) == "foo";
  }
  {
    p::MessageData data;
    p::Value expr = reader.send("parse", p::Array::of("(foo bar baz)"), &data);
    assert is<p::Array>(expr);
    p::Array list = cast<p::Array>(expr);
    assert list.length() == 3;
    assert cast<p::String>(list[0]) == "foo";
    assert cast<p::String>(list[1]) == "bar";
    assert cast<p::String>(list[2]) == "baz";
  }
  {
    p::MessageData data;
    p::Value expr = reader.send("parse", p::Array::of("(foo (bar baz) quux)"),
        &data);
    assert is<p::Array>(expr);
    p::Array list = cast<p::Array>(expr);
    assert list.length() == 3;
    assert cast<p::String>(list[0]) == "foo";
    p::Array inner = cast<p::Array>(list[1]);
    assert inner.length() == 2;
    assert cast<p::String>(inner[0]) == "bar";
    assert cast<p::String>(inner[1]) == "baz";
    assert cast<p::String>(list[2]) == "quux";
  }
}

TEST(write) {
  Write::load();
  p::Object reader = p::ServiceRegistry::lookup("neutrino.io.read");
  assert !reader.is_empty();
  p::Object writer = p::ServiceRegistry::lookup("neutrino.io.write");
  assert !writer.is_empty();
  p::MessageData data;
  p::Value expr = reader.send("parse", p::Array::of(" ( a ( b c ) d ( e ( f ) ) ) "),
      &data);
  assert !expr.is_empty();
  p::Value written = writer.send("unparse", p::Array::of(expr), &data);
  assert cast<p::String>(written) == "(a (b c) d (e (f)))";
}

/*
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
*/
