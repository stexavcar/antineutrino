#ifndef _STRING_INL_H
#define _STRING_INL_H

#include "utils/array-inl.h"
#include "utils/checks.h"

namespace neutrino {

array<const char> string::chars() {
  return NEW_ARRAY(chars_, length_);
}

uword string::operator[](uword index) const {
  ASSERT(index < length());
  return chars_[index];
}

string string::substring(uword start) {
  ASSERT(start < length());
  return string(chars_ + start, length_ - start);
}

string string::substring(uword start, uword length) {
  if (length == 0) return string();
  ASSERT(start + length < length_);
  return string(chars_ + start, length);
}

class scoped_string {
public:
  scoped_string(string str) : value_(str) { }
  ~scoped_string() { value_.dispose(); }
  string &operator*() { return value_; }
  string *operator->() { return &value_; }
  const array<const char> chars() { return value_.chars(); }
private:
  string value_;
};

class c_string {
public:
  c_string(string str) : value_(string::dup(str)) { }
  ~c_string() { value_.dispose(); }
  const char *operator*() { return value_.chars().start(); }
private:
  string value_;
};

} // namespace neutrino

#endif // _STRING_INL_H
