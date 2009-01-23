// S-expression reader following a subset of Rivest's draft RFC.
// See http://people.csail.mit.edu/rivest/Sexp.txt.

#ifndef _IO_READ
#define _IO_READ

#include "utils/arena.h"
#include "utils/string.h"

namespace neutrino {

class SexpScanner {
public:
  enum Token { tLeft, tRight, tString, tEnd, tError };
  SexpScanner(string str, Arena &arena);
  Token next();
  vector<uint8_t> last_string() { return last_string_; }

private:
  vector<uint8_t> scan_token();
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
  vector<uint8_t> last_string_;
};

class s_exp {
public:
  enum Type { seString, seList };
  virtual Type type() = 0;
  static s_exp *read(string str, Arena &arena);
};

class s_string : public s_exp {
public:
  s_string(vector<uint8_t> chars) : chars_(chars) { }
  virtual Type type() { return seString; }
  vector<uint8_t> chars() { return chars_; }
  static const Type kTypeTag = seString;
private:
  vector<uint8_t> chars_;
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

template <typename T>
static inline bool is(s_exp *exp);

template <typename T>
static inline T *cast(s_exp *exp);

} // namespace neutrino

#endif // _IO_READ
