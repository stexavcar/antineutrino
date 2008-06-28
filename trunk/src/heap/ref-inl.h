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
inline persistent<C> RefStack::new_persistent(C *obj) {
  uword index = persistent_list().length();
  persistent_cell *cell = new persistent_cell(obj, *this, index);
  persistent_list().push(cell);
  return persistent<C>(cell);
}

template <class C>
C* abstract_ref<C>::operator->(){
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

base_stack_ref_block::base_stack_ref_block(RefStack &refs)
    : refs_(refs)
    , count_(0)
    , prev_(refs.top()) {
  refs.set_top(this);
}

base_stack_ref_block::~base_stack_ref_block() {
  refs_.set_top(prev_);
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

ref_iterator::ref_iterator(RefStack &refs)
    : refs_(refs) {
  current_block_ = 0;
  if (refs.block_stack().length() > 0) {
    ref_block *bottom = refs.block_stack()[0];
    current_ = bottom->first_cell();
    limit_ = bottom->limit();
  } else {
    current_ = refs.current().next_cell;
    limit_ = current_;
  }
}

bool ref_iterator::has_next() {
  return current_ != refs().current().next_cell;
}

Value *&ref_iterator::next() {
  if (current_ == limit_) {
    ref_block *next = refs().block_stack()[++current_block_];
    current_ = next->first_cell();
    limit_ = next->limit();
    ASSERT(current_ < limit_);
  }
  return *(current_++);
}

persistent_iterator::persistent_iterator(RefStack &refs)
    : refs_(refs), index_(0) { }

bool persistent_iterator::has_next() {
  return index_ < refs().persistent_list().length();
}

Value *&persistent_iterator::next() {
  persistent_cell *cell = refs().persistent_list()[index_++];
  return *cell->cell();
}

} // neutrino

#endif // _HEAP_HANDLE_INL
