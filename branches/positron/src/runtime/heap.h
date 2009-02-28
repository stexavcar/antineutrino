#ifndef _RUNTIME_HEAP
#define _RUNTIME_HEAP

#include "utils/array.h"
#include "utils/global.h"
#include "utils/string.h"
#include "value/condition.h"
#include "value/value.h"

#define eAllocators(VISIT)                                                                                                     \
  VISIT(String,     new_string,      (word length),                         (word length),                 (length))           \
  VISIT(String,     new_string,      (string str),                          (string str),                  (str))              \
  VISIT(Array,      new_array,       (word length),                         (word length),                 (length))           \
  VISIT(Blob,       new_blob,        (word length),                         (word length),                 (length))           \
  VISIT(SyntaxTree, new_syntax_tree, (ref<Blob> code, ref<Array> literals), (Blob *code, Array *literals), (*code, *literals)) \
  VISIT(Nil,        new_nil,         (),                                    (),                            ())                 \
  VISIT(HashMap,    new_hash_map,    (),                                    (),                            ())

namespace neutrino {

template <word L>
class Page : public nocopy {
public:
  Page();
  uint8_t *cursor() { return cursor_; }
  void set_cursor(uint8_t *v) { cursor_ = v; }
  uint8_t *limit() { return limit_; }
  array<uint8_t> &memory() { return memory_; }
  void zap(word filler);
  static const word kBeforeZapValue = IF_ELSE(cc64, 0xDeeeeeadBeeeeeef, 0xDeadBeef);
  static const word kAfterZapValue = IF_ELSE(cc64, 0xFeeeeeadBeeeeeee, 0xFeedBeee);
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
  void zap() { page().zap(Page<kPageSize>::kAfterZapValue); }
  static inline uint8_t *align(uint8_t *ptr);
  static inline word align(word size);
private:
  static const word kAlignment = kPointerSize;
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
  probably initialize();
  array<uint8_t> allocate(size_t size);
#define MAKE_ALLOCATOR(Type, name, safe_params, raw_params, args)    \
  allocation<Type> name raw_params;
eAllocators(MAKE_ALLOCATOR)
#undef MAKE_ALLOCATOR
  probably collect_garbage();
  Space &space() { return *space_; }
private:
  Runtime &runtime() { return runtime_; }
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
