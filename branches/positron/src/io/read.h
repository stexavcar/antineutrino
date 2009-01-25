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
  virtual bool match(const pattern &m) = 0;
  static s_exp *read(string str, Arena &arena);
  static p::Object create_service();
};

class s_string : public s_exp {
public:
  s_string(vector<uint8_t> chars) : chars_(chars) { }
  virtual Type type() { return seString; }
  virtual bool match(const pattern &m);
  vector<uint8_t> chars() { return chars_; }
  static const Type kTypeTag = seString;
private:
  vector<uint8_t> chars_;
};

class s_list : public s_exp {
public:
  s_list(vector<s_exp*> elements) : elements_(elements) { }
  virtual Type type() { return seList; }
  virtual bool match(const pattern &m);
  word length() { return elements().length(); }
  s_exp *get(word n) { return elements()[n]; }
  static const Type kTypeTag = seList;
private:
  vector<s_exp*> elements() { return elements_; }
  vector<s_exp*> elements_;
};

class pattern {
public:
  virtual bool match_string(s_string *that) const { return false; }
  virtual bool match_list(s_list *that) const { return false; }
};

class m_none : public pattern {

};

class m_string : public pattern {
public:
  m_string(const string &str) : str_(str), str_ptr_(NULL) { }
  m_string(vector<uint8_t> *str_ptr) : str_ptr_(str_ptr) { }
  virtual bool match_string(s_string *that) const;
private:
  const string &str() const { return str_; }
  vector<uint8_t> *str_ptr() const { return str_ptr_; }
  string str_;
  vector<uint8_t> *str_ptr_;
};

class m_list : public pattern {
public:
  m_list(const pattern &e0 = m_none(), const pattern &e1 = m_none(),
      const pattern &e2 = m_none(), const pattern &e3 = m_none(),
      const pattern &e4 = m_none(), const pattern &e5 = m_none())
    : e0_(e0), e1_(e1), e2_(e2), e3_(e3), e4_(e4), e5_(e5) { }
  virtual bool match_list(s_list *that) const;
private:
  const pattern &e0_;
  const pattern &e1_;
  const pattern &e2_;
  const pattern &e3_;
  const pattern &e4_;
  const pattern &e5_;
};

template <typename T>
static inline bool is(s_exp *exp);

template <typename T>
static inline T *cast(s_exp *exp);

} // namespace neutrino

#endif // _IO_READ
