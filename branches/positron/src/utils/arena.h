#ifndef _UTILS_ARENA
#define _UTILS_ARENA

#include "utils/buffer.h"
#include "utils/global.h"

namespace neutrino {

class ArenaBlock {
public:
  ArenaBlock(word size);
  ~ArenaBlock();
  uint8_t *start() { return data().start(); }
  uint8_t *end() { return data().end(); }
  word size() { return data().length(); }
private:
  vector<uint8_t> data() { return data_.as_vector(); }
  own_vector<uint8_t> data_;
};

class Arena : public nocopy {
public:
  Arena();
  ~Arena();
  inline void *allocate(size_t size);
  void *extend(size_t size);

private:
  static const word kInitialBlockSize = 8 KB;
  uint8_t *cursor() { return cursor_; }
  uint8_t *limit() { return limit_; }
  vector<ArenaBlock*> blocks() { return blocks_.as_vector(); }
  uint8_t *cursor_;
  uint8_t *limit_;
  own_buffer<ArenaBlock> blocks_;
};

} // namespace neutrino

#endif // _UTILS_ARENA
