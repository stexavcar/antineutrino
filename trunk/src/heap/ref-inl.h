#ifndef _HEAP_REF_INL
#define _HEAP_REF_INL

#include "heap/ref.h"
#include "utils/checks.h"

namespace neutrino {

template <class C> static inline ref<C> open(ref_traits<C> *that) {
  return *reinterpret_cast<ref<C>*>(that);
}

template <class T>
T **RefScope::new_cell(T *value) {
  Value **result = current().next_cell;
  if (result == current().limit) result = grow();
  else current().next_cell = result + 1;
  *result = value;
  return reinterpret_cast<T**>(result);
}

template <class C>
static inline ref<C> new_ref(C *obj) {
  return RefScope::new_cell(obj);
}

template <class C>
static inline permanent<C> new_permanent(C *obj) {
  C **cell = new C*;
  *cell = obj;
  return permanent<C>(cell);
}

template <class C>
C* abstract_ref<C>::operator->() {
  ASSERT(cell_ != 0);
  return *cell_;
}

template <class C>
C* abstract_ref<C>::operator*() {
  return operator->();
}

template <class C>
ref<C> abstract_ref<C>::empty() {
  return ref<C>(static_cast<C**>(0));
}

RefScopeInfo::RefScopeInfo()
    : block_count(-1)
    , next_cell(NULL)
    , limit(NULL) { }

RefScope::RefScope() : previous_(current_) {
  current().block_count = 0;
}

RefScope::~RefScope() {
  if (current().block_count > 0) shrink();
  current_ = previous_;
}

}

#endif // _HEAP_HANDLE_INL
