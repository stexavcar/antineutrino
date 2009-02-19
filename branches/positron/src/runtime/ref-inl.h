#ifndef _RUNTIME_REF_INL
#define _RUNTIME_REF_INL

#include "runtime/ref.h"
#include "utils/check.h"
#include "utils/buffer-inl.h"

namespace neutrino {

template <class C>
inline persistent<C> RefManager::new_persistent(C *obj) {
  word index = persistent_list().length();
  persistent_cell *cell = new persistent_cell(obj, *this, index);
  persistent_list().push(cell);
  return persistent<C>(cell);
}


template <word n>
template<typename T>
inline ref<T> ref_block<n>::operator()(T *val) {
  word index = count_++;
  assert index < n;
  entries_[index] = val;
  return ref<T>(&entries_[index]);
}


array<Value*> abstract_ref_block::entries() {
  ref_block<>* self = static_cast<ref_block<>*>(this);
  return TO_ARRAY(Value*, self->entries_, self->size_);
}


template <class C>
C* ref<C>::operator->(){
  assert cell() != static_cast<void*>(NULL);
  return static_cast<C*>(*cell());
}


template <class C>
C* ref<C>::operator*() {
  return operator->();
}


template <class C>
ref<C> ref<C>::empty() {
  return ref<C>(static_cast<C**>(0));
}


abstract_ref_block::abstract_ref_block(RefManager &refs)
    : count_(0)
    , refs_(refs)
    , prev_(refs.top()) {
  refs.set_top(this);
}


abstract_ref_block::~abstract_ref_block() {
  refs_.set_top(prev_);
}


ref_iterator::ref_iterator(RefManager &refs)
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


persistent_iterator::persistent_iterator(RefManager &refs)
    : refs_(refs), index_(0) { }


bool persistent_iterator::has_next() {
  return index_ < refs().persistent_list().length();
}


Value *&persistent_iterator::next() {
  persistent_cell *cell = refs().persistent_list()[index_++];
  return *cell->cell();
}


} // neutrino

#endif // _RUNTIME_REF_INL
