#ifndef _SPACE
#define _SPACE

#include "utils/types.h"

namespace neutrino {

class SemiSpace {
public:
  SemiSpace(uword capacity);
  ~SemiSpace();
  uword capacity();
  address start() { return data_; }
  address end() { return data_ + capacity_; }
  address cursor() { return data_ + cursor_; }
  uword bytes_allocated() { return cursor() - start(); }
  bool contains(Object *obj);
  inline address allocate(uword size);
private:
  friend class SemiSpaceIterator;
  address data_;
  uword cursor_;
  uword capacity_;
};

class SemiSpaceIterator {
public:
  inline SemiSpaceIterator(SemiSpace &space);
  inline bool has_next();
  inline Object *next();
private:
  SemiSpace &space_;
  uword offset_;
};

}

#endif // _SPACE
