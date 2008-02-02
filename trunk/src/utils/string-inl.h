#ifndef _STRING_INL_H
#define _STRING_INL_H

#include "utils/checks.h"

namespace neutrino {

uword string::operator[](uword index) {
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

}

#endif // _STRING_INL_H
