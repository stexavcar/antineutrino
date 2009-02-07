// S-expression reader following a subset of Rivest's draft RFC.
// See http://people.csail.mit.edu/rivest/Sexp.txt.

#ifndef _IO_READ
#define _IO_READ

#include "io/miniheap.h"
#include "utils/arena.h"
#include "utils/string.h"

namespace neutrino {

class SexpScanner {
public:
  enum Token { tLeft, tRight, tString, tNumber, tEnd, tError };
  SexpScanner(p::String str, Arena &arena);
  Token next();
  string last_string() { return last_string_; }
  word last_number() { return last_number_; }

private:
  string scan_token();
  word scan_number();
  void advance();
  void skip_whitespace();

  Arena &arena() { return arena_; }
  p::String &str() { return str_; }
  word cursor() { return cursor_; }
  code_point current() { return current_; }
  Arena &arena_;
  p::String str_;
  word cursor_;
  bool at_end_;
  code_point current_;
  string last_string_;
  word last_number_;
};

class Reader {
public:
  static p::Value read(Factory &factory, p::String input);
};

template <> struct coerce<SexpScanner::Token> { typedef word type; };

} // namespace neutrino

#endif // _IO_READ
