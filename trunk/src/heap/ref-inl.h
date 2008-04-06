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
T **RefStack::new_cell(T *value) {
  Value **result = current().next_cell;
  if (result == current().limit) result = grow();
  else current().next_cell = result + 1;
  *result = value;
  return reinterpret_cast<T**>(result);
}

template <class C>
ref<C> RefStack::new_ref(C *obj) {
  return ref<C>(new_cell(obj));
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

ref_scope_info::ref_scope_info()
    : block_count(-1)
    , next_cell(NULL)
    , limit(NULL) { }

ref_scope::ref_scope(RefStack &manager)
    : previous_(manager.current())
    , manager_(manager) {
  manager.current().block_count = 0;
}

ref_scope::~ref_scope() {
  if (manager().current().block_count > 0) manager().shrink();
  manager().current_ = previous_;
}

ref_iterator::ref_iterator(RefStack &manager) 
    : manager_(manager) {
  current_block_ = 0;
  if (manager.block_stack().length() > 0) {
    ref_block *bottom = manager.block_stack()[0];
    current_ = bottom->first_cell();
    limit_ = bottom->limit();
  } else {
    current_ = manager.current().next_cell;
    limit_ = current_;
  }
}

bool ref_iterator::has_next() {
  return current_ != manager().current().next_cell;
}

Value *&ref_iterator::next() {
  if (current_ == limit_) {
    ref_block *next = manager().block_stack()[++current_block_];
    current_ = next->first_cell();
    limit_ = next->limit();
    ASSERT(current_ < limit_);
  }
  return *(current_++);
}

} // neutrino

#endif // _HEAP_HANDLE_INL
