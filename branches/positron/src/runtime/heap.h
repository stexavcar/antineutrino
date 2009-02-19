#ifndef _RUNTIME_HEAP
#define _RUNTIME_HEAP

#include "utils/array.h"
#include "utils/global.h"

namespace neutrino {

template <word L>
class Page : public nocopy {
public:
  Page()
    : cursor_(memory().start())
    , limit_(memory().start() + L) { }
  uint8_t *cursor() { return cursor_; }
  void set_cursor(uint8_t *v) { cursor_ = v; }
  uint8_t *limit() { return limit_; }
private:
  array<uint8_t> &memory() { return memory_; }
  embed_array<uint8_t, L> memory_;
  uint8_t *cursor_;
  uint8_t *limit_;
};

class Space : public nocopy {
public:
  array<uint8_t> allocate(size_t size);
private:
  static const word kPageSize = 8 KB;
  Page<kPageSize> &page() { return page_; }
  Page<kPageSize> page_;
};

class Heap : public nocopy {
public:
  Heap(Runtime &runtime)
    : runtime_(runtime)
    , space_(one_)
    , other_(two_) { }
  array<uint8_t> allocate(size_t size);
  void collect_garbage();
private:
  Runtime &runtime() { return runtime_; }
  Runtime &runtime_;
  Space &space() { return space_; }
  Space &other() { return other_; }
  Space one_;
  Space two_;
  Space &space_;
  Space &other_;
};

} // namespace neutrino

#endif // _RUNTIME_HEAP
