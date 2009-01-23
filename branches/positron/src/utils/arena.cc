#include "utils/arena-inl.h"
#include "utils/check-inl.h"

namespace neutrino {

ArenaBlock::ArenaBlock(word size)
  : data_(vector<uint8_t>::allocate(size)) {
}

ArenaBlock::~ArenaBlock() { }

Arena::Arena() : cursor_(NULL), limit_(NULL) { }

void *Arena::extend(size_t size) {
  assert (cursor() + size) > limit();
  vector<ArenaBlock*> earlier = blocks();
  word block_size = (earlier.is_empty())
    ? kInitialBlockSize
    : grow_value(earlier.last()->size());
  ArenaBlock *next = new ArenaBlock(block_size);
  blocks_.append(next);
  cursor_ = next->start() + size;
  limit_ = next->end();
  return next->start();
}

Arena::~Arena() { }

} // namespace neutrino
