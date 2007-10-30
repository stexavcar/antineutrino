#ifndef _ZONE
#define _ZONE

#include <new>

#include "utils/globals.h"
#include "utils/string.h"

namespace neutrino {
namespace zone {

/**
 * Zone object is the superclass of all zone-allocated objects.  If
 * an object inherits from ZoneObject allocation will take place in
 * the topmost allocation zone.
 */
class Object {
public:
  static inline void *operator new(size_t size);
};

template <class T>
class Vector : public Object {
public:
  Vector(uint32_t length);
  T &at(uint32_t index) { return this->operator[](index); }
  T &operator[](uint32_t index);
  uint32_t length() { return length_; }
private:
  uint32_t length_;
  T *elms_;
};

/**
 * An arena is a single contiguous chunk of memory.  A zone maintains
 * a number of arenas and whenever it exhausts the memory in one it
 * allocates a new one.  The arenas within one zone are chained.
 */
class Arena {
public:
  Arena(Arena *prev) : prev_(prev), cursor_(0) { }
  int8_t *allocate(size_t size);
  Arena *prev() { return prev_; }
  size_t bytes_allocated() { return cursor_; }
  static const size_t kSize = 16 KB;
private:
  int8_t memory_[kSize];
  Arena *prev_;
  size_t cursor_;
};

/**
 * A zone allows objects to be allocated within an arena that can be
 * deallocated in one go, rather than one object at a time.
 */
class Zone {
public:
  Zone();
  ~Zone();
  int8_t *allocate(size_t size);
  static inline Zone &current();
  size_t bytes_allocated();
private:
  Arena *arena_;
  Zone *prev_;
  static Zone *current_;
};

}
}

#endif // _ZONE
