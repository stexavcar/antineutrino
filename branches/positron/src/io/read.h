// S-expression reader following a subset of Rivest's draft RFC.
// See http://people.csail.mit.edu/rivest/Sexp.txt.

#ifndef _IO_READ
#define _IO_READ

#include "utils/arena.h"
#include "utils/string.h"

namespace neutrino {

struct Location {
  word start, end;
};

class SexpScanner {
public:
  enum Token {
    tLeft,
    tRight,
    tString,
    tEnd,
    tError,
    tNumber,
    tLastToken
  };
  SexpScanner(string str, Arena &arena);
  Token next();
  string last_string() { return last_string_; }
  word last_number() { return last_number_; }
  Location location() { return location_; }

private:
  string scan_token();
  word scan_number();
  void advance();
  void skip_whitespace();

  Arena &arena() { return arena_; }
  string &str() { return str_; }
  word cursor() { return pos_ - 1; }
  code_point current() { return current_; }
  Arena &arena_;
  string str_;
  word pos_;
  code_point current_;
  string last_string_;
  word last_number_;
  Location location_;
};

class parse_result {
public:
  parse_result(s_exp *value) : value_(value) { }
  bool is_failure() { return value_ == NULL; }
  s_exp *value() { return value_; }
private:
  s_exp *value_;
};



class SexpParser {
public:
  SexpParser(string input, Arena &arena);
  s_exp *read();
  Location error_location() { return error_location_; }
private:
  s_exp *record_error();
  void advance();
  parse_result parse_exp();
  SexpScanner &scan() { return scan_; }
  Arena &arena() { return arena_; }
  SexpScanner::Token current() { return current_; }
  SexpScanner scan_;
  Arena &arena_;
  SexpScanner::Token current_;
  Location error_location_;
};

class s_exp {
public:
  enum Type { seString, seList, seNumber };
  virtual Type type() = 0;
};

class s_string : public s_exp {
public:
  s_string(string chars) : chars_(chars) { }
  virtual Type type() { return seString; }
  string chars() { return chars_; }
  static const Type kTypeTag = seString;
private:
  string chars_;
};

class s_number : public s_exp {
public:
  s_number(word value) : value_(value) { }
  virtual Type type() { return seNumber; }
  word value() { return value_; }
  static const Type kTypeTag = seNumber;
private:
  word value_;
};

class s_list : public s_exp {
public:
  s_list(vector<s_exp*> elements) : elements_(elements) { }
  virtual Type type() { return seList; }
  word length() { return elements().length(); }
  s_exp *get(word n) { return elements()[n]; }
  static const Type kTypeTag = seList;
private:
  vector<s_exp*> elements() { return elements_; }
  vector<s_exp*> elements_;
};

template <> struct coerce<SexpScanner::Token> { typedef word type; };

template <typename T>
static inline bool is(s_exp *exp);

template <typename T>
static inline T *cast(s_exp *exp);

} // namespace neutrino

#endif // _IO_READ
