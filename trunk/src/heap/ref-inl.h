#ifndef _HEAP_REF_INL
#define _HEAP_REF_INL

#include "heap/ref.h"
#include "utils/checks.h"
#include "utils/list-inl.h"

namespace neutrino {


template <class C> static inline ref<C> open(ref_traits<C> *that) {
  return *reinterpret_cast<ref<C>*>(that);
}


template <class C>
inline persistent<C> RefStack::new_persistent(C *obj) {
  uword index = persistent_list().length();
  persistent_cell *cell = new persistent_cell(obj, *this, index);
  persistent_list().push(cell);
  return persistent<C>(cell);
}


template <uword n>
template<typename T>
inline ref<T> stack_ref_block<n>::operator()(T *val) {
  uword index = count_++;
  ASSERT_LT_C(cnRefOverflow, index, n);
  entries_[index] = val;
  return ref<T>(reinterpret_cast<T**>(&entries_[index]));
}


array<Value*> base_stack_ref_block::entries() {
  stack_ref_block<>* self = static_cast<stack_ref_block<>*>(this);
  return NEW_ARRAY(self->entries_, self->size_);
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
    : count_(0)
    , refs_(refs)
    , prev_(refs.top()) {
  refs.set_top(this);
}


base_stack_ref_block::~base_stack_ref_block() {
  refs_.set_top(prev_);
}


ref_iterator::ref_iterator(RefStack &refs)
    : current_(refs.top())
    , next_index_(0) {
  advance();
}


bool ref_iterator::has_next() {
  return current_ != NULL;
}


Value *&ref_iterator::next() {
  Value *&result = current_->entries()[next_index_ - 1];
  advance();
  return result;
}


void ref_iterator::advance() {
  // Loop around until we find a block with
  while (current_ != NULL) {
    if (next_index_ < current_->count()) {
      // If there are more values in this block we just advance
      // to the next value.
      next_index_++;
      return;
    } else {
      // Otherwise new go on to the next block and loop around.
      next_index_ = 0;
      current_ = current_->prev();
    }
  }
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
