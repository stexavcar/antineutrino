#ifndef _RUNTIME_HEAP
#define _RUNTIME_HEAP

#include "utils/array.h"
#include "utils/global.h"
#include "utils/string.h"
#include "value/condition.h"
#include "value/value.h"

#define eAllocators(VISIT)                                           \
  VISIT(String, new_string, (word length), (length))                 \
  VISIT(String, new_string, (string str), (str))

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
  array<uint8_t> &memory() { return memory_; }
private:
  embed_array<uint8_t, L> memory_;
  uint8_t *cursor_;
  uint8_t *limit_;
};

class Space : public nocopy {
public:
  array<uint8_t> allocate(size_t size);
  uint8_t *start() { return page().memory().start(); }
  uint8_t *limit() { return page().cursor(); }
private:
  static const word kPageSize = 8 KB;
  Page<kPageSize> &page() { return page_; }
  Page<kPageSize> page_;
};

class SpaceIterator : public nocopy {
public:
  inline SpaceIterator(Space &space);
  inline bool has_next();
  inline Object *next();
private:
  Space &space_;
  uint8_t *cursor_;
};

class Heap : public nocopy {
public:
  Heap(Runtime &runtime)
    : runtime_(runtime)
    , space_(NULL) { }
  likely<> initialize();
  array<uint8_t> allocate(size_t size);
#define MAKE_ALLOCATOR(Type, name, params, args)                     \
  allocation<Type> name params;
eAllocators(MAKE_ALLOCATOR)
#undef MAKE_ALLOCATOR
  likely<> collect_garbage();
private:
  Runtime &runtime() { return runtime_; }
  Space &space() { return *space_; }
  Runtime &runtime_;
  Space *space_;
};

class FieldMigrator {
public:
  FieldMigrator(Space &from_space, Space &to_space)
    : from_space_(from_space)
    , to_space_(to_space) { }
  void migrate_field(Value **field);
private:
  Space &from_space() { return from_space_; }
  Space &to_space() { return to_space_; }
  Space &from_space_;
  Space &to_space_;
};

} // namespace neutrino

#endif // _RUNTIME_HEAP
