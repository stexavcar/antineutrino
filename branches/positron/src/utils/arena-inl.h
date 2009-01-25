#ifndef _UTILS_ARENA_INL
#define _UTILS_ARENA_INL

#include "utils/arena.h"

namespace neutrino {

inline void *Arena::allocate(size_t size) {
  uint8_t *start = cursor();
  if (start + size <= limit()) {
    cursor_ += size;
    return start;
  } else {
    return extend(size);
  }
}

template <typename T>
class arena_array_allocator {
public:
  arena_array_allocator(Arena &arena) : arena_(arena) { }
  array<T> allocate(size_t size) { return TO_ARRAY(T, new (arena()) T[size], size); }
  void dispose(array<T> obj) { /* ignore */ }
private:
  Arena &arena() { return arena_; }
  Arena &arena_;
};

template <typename T>
class arena_buffer : public buffer< T, arena_array_allocator<T> > {
public:
  arena_buffer(Arena &arena) : buffer< T, arena_array_allocator<T> >(arena_array_allocator<T>(arena)) { }
};

} // namespace neutrino

inline void *operator new(size_t size, neutrino::Arena &arena) {
  return arena.allocate(size);
}

inline void *operator new[](size_t size, neutrino::Arena &arena) {
  return arena.allocate(size);
}

#endif // _UTILS_ARENA_INL
