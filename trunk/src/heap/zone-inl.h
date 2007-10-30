#ifndef _ZONE_INL
#define _ZONE_INL

#include "heap/zone.h"
#include "utils/checks.h"

namespace neutrino {
namespace zone {

Zone &Zone::current() {
  ASSERT(current_ != NULL);
  return *current_;
}

void *Object::operator new(size_t size) {
  return Zone::current().allocate(size);
}

template <class T>
Vector<T>::Vector(uint32_t length) {
  length_ = length;
  elms_ = reinterpret_cast<T*>(Zone::current().allocate(sizeof(T) * length));
}

template <class T>
T &Vector<T>::operator[](uint32_t index) {
  ASSERT(index < length_);
  return elms_[index];
}

}
}

#endif // _ZONE_INL
