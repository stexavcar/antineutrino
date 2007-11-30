#ifndef _SPACE
#define _SPACE

#include "utils/types.h"

namespace neutrino {

class SemiSpace {
public:
  SemiSpace(uint32_t capacity);
  ~SemiSpace();
  uint32_t capacity();
  address start() { return data_; }
  address end() { return data_ + capacity_; }
  address cursor() { return data_ + cursor_; }
  bool contains(address addr);
  inline address allocate(uint32_t size);
private:
  friend class SemiSpaceIterator;
  address data_;
  uint32_t cursor_;
  uint32_t capacity_;
};

class SemiSpaceIterator {
public:
  inline SemiSpaceIterator(SemiSpace &space);
  inline bool has_next();
  inline Object *next();
private:
  SemiSpace &space_;
  uint32_t offset_;
};

}

#endif // _SPACE
