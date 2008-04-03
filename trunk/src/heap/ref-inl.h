#ifndef _HEAP_REF_INL
#define _HEAP_REF_INL

#include "heap/ref.h"
#include "utils/checks.h"
#include "utils/list-inl.h"

namespace neutrino {

template <class C> static inline ref<C> open(ref_traits<C> *that) {
  return *reinterpret_cast<ref<C>*>(that);
}

template <class T>
T **ref_scope::new_cell(T *value) {
  Value **result = current().next_cell;
  if (result == current().limit) result = grow();
  else current().next_cell = result + 1;
  *result = value;
  return reinterpret_cast<T**>(result);
}

template <class C>
static inline ref<C> new_ref(C *obj) {
  return ref_scope::new_cell(obj);
}

template <class C>
static inline persistent<C> new_persistent(C *obj) {
  C **cell = new C*;
  *cell = obj;
  return persistent<C>(cell);
}

template <class C>
C* abstract_ref<C>::operator->() {
  ASSERT(cell() != 0);
  return *cell();
}

template <class C>
C* abstract_ref<C>::operator*() {
  return operator->();
}

template <class C>
ref<C> abstract_ref<C>::empty() {
  return ref<C>(static_cast<C**>(0));
}

ref_scopeInfo::ref_scopeInfo()
    : block_count(-1)
    , next_cell(NULL)
    , limit(NULL) { }

ref_scope::ref_scope() : previous_(current_) {
  current().block_count = 0;
}

ref_scope::~ref_scope() {
  if (current().block_count > 0) shrink();
  current_ = previous_;
}

RefIterator::RefIterator() {
  current_block_ = 0;
  if (ref_scope::block_stack().length() > 0) {
    RefBlock *bottom = ref_scope::block_stack()[0];
    current_ = bottom->first_cell();
    limit_ = bottom->limit();
  } else {
    current_ = ref_scope::current().next_cell;
    limit_ = current_;
  }
}

bool RefIterator::has_next() {
  return current_ != ref_scope::current().next_cell;
}

Value *&RefIterator::next() {
  if (current_ == limit_) {
    RefBlock *next = ref_scope::block_stack()[++current_block_];
    current_ = next->first_cell();
    limit_ = next->limit();
    ASSERT(current_ < limit_);
  }
  return *(current_++);
}

}

#endif // _HEAP_HANDLE_INL
