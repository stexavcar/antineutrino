#ifndef _STRING_INL_H
#define _STRING_INL_H

#include "utils/checks.h"

namespace neutrino {

uint32_t string::operator[](uint32_t index) {
  ASSERT(index < length());
  return chars_[index];
}

string string::substring(uint32_t start) {
  ASSERT(start < length());
  return string(chars_ + start, length_ - start);
}

}

#endif // _STRING_INL_H
