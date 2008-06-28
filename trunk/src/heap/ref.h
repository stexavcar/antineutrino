#ifndef _HANDLE_
#define _HANDLE_

#include "utils/list.h"
#include "utils/types.h"
#include "monitor/monitor.h"

namespace neutrino {


// --- R e f   S c o p e ---

class base_stack_ref_block {
public:
  inline base_stack_ref_block(RefStack &refs);
  inline ~base_stack_ref_block();
  uword count() { return count_; }
  base_stack_ref_block *prev() { return prev_; }
  inline array<Value*> entries();
protected:
  uword count_;
private:
  RefStack &refs_;
  base_stack_ref_block *prev_;
};


template <uword size = 16>
class stack_ref_block : public base_stack_ref_block {
public:
  inline stack_ref_block(RefStack &refs)
    : base_stack_ref_block(refs)
#ifdef DEBUG
    , size_(size)
#endif // DEBUG
    { }
  template <typename T> inline ref<T> operator()(T *val);
private:
  friend class base_stack_ref_block;
  IF_DEBUG(uword size_);
  Value *entries_[size];
};


/**
 * A stack of references.
 */
class RefStack {
public:
  RefStack();

  template <class C> inline persistent<C> new_persistent(C *obj);

  base_stack_ref_block *top() { return top_; }
  void set_top(base_stack_ref_block *value) { top_ = value; }

private:
  friend class ref_iterator;
  friend class persistent_iterator;
  template <class C> friend class persistent;

  void dispose(persistent_cell &cell);

  base_stack_ref_block *top_;

  list_buffer<persistent_cell*> &persistent_list() { return persistent_list_; }
  list_buffer<persistent_cell*> persistent_list_;
};


class ref_iterator {
public:
  inline ref_iterator(RefStack &refs);
  inline bool has_next();
  inline Value *&next();
private:
  inline void advance();
  base_stack_ref_block *current_;
  uword next_index_;
};


class persistent_iterator {
public:
  inline persistent_iterator(RefStack &refs);
  inline bool has_next();
  inline Value *&next();
private:
  RefStack &refs() { return refs_; }
  RefStack &refs_;
  uword index_;
};


// ---------------------------
// --- R e f e r e n c e s ---
// ---------------------------

/**
 * An abstract superclass used to hold the behavior shared between
 * instantiations of ref.
 */
template <class C>
class abstract_ref {
public:
  inline explicit abstract_ref(C **cell = 0) : cell_(cell) { }
  template <class D> inline abstract_ref(abstract_ref<D> that)
      : cell_(reinterpret_cast<C**>(that.cell())) { }
  inline C* operator*();
  inline C* operator->();
  inline C** cell() { return cell_; }
  template <class D>
  bool operator==(abstract_ref<D> that) { return this->operator*() == *that; }
  inline bool is_empty() { return cell_ == 0; }
  static inline ref<C> empty();
protected:
  C **cell_;
};

/**
 * This macro generates the constructors for the specializations of
 * Handle defined for the different object types.
 */
#define REF_CLASS_BODY(T)                                            \
  inline ref(T **cell = 0) : abstract_ref<T>(cell) { }               \
  template <class D>                                                 \
  inline ref(ref<D> that) : abstract_ref<T>(that) {                  \
    TYPE_CHECK(T*, D*);                                              \
  }

#define DEFINE_REF_CLASS(Type)                                       \
  template <> class ref<Type> : public ref_traits<Type>              \
                              , public abstract_ref<Type> {          \
public:                                                              \
  REF_CLASS_BODY(Type)                                               \
}

/**
 * Instantiations of this class hold the ref methods for the parameter
 * class.  The methods are then added to the ref class for the type
 * using the DEFINE_REF_CLASS.
 */
template <class C> class ref_traits { };

template <class C> static inline ref<C> open(ref_traits<C> *that);

template <class C>
class ref : public abstract_ref<C> {
public:
  REF_CLASS_BODY(C)
};

class persistent_cell {
public:
  persistent_cell(Value *value, RefStack &refs, uword index)
    : cell_(value)
    , refs_(refs)
    , index_(index) { }
  Value **cell() { return &cell_; }
  uword index() { return index_; }
  void set_index(uword value) { index_ = value; }
  void dispose();
private:
  template <class C> friend class persistent;
  Value *cell_;
  RefStack &refs_;
  uword index_;
};

template <class C>
class persistent : public ref<C> {
public:
  inline persistent(persistent_cell *storage)
    : ref<C>(reinterpret_cast<C**>(storage->cell()))
    , storage_(storage) { }
  inline persistent() : ref<C>(), storage_(0) { }
  inline void dispose() { if (storage_) storage_->refs_.dispose(*storage_); }
private:
  persistent_cell *storage_;
};

}

#endif // _HANDLE_
