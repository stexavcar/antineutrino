#ifndef _RUNTIME_REF
#define _RUNTIME_REF

#include "utils/buffer.h"
#include "utils/types.h"

namespace neutrino {

class abstract_ref_block : public nocopy {
public:
  inline abstract_ref_block(RefManager &refs);
  inline ~abstract_ref_block();
  word count() { return count_; }
  abstract_ref_block *prev() { return prev_; }
  inline array<Value*> entries();
protected:
  word count_;
private:
  RefManager &refs_;
  abstract_ref_block *prev_;
};

template <word size = 16>
class ref_block : public abstract_ref_block {
public:
  inline ref_block(RefManager &refs)
    : abstract_ref_block(refs)
#ifdef DEBUG
    , size_(size)
#endif // DEBUG
    { }
  template <typename T> inline ref<T> operator()(T *val);
private:
  friend class abstract_ref_block;
  IF(ccDebug, word size_);
  Value *entries_[size];
};


/**
 * A stack of references.
 */
class RefManager : public nocopy {
public:
  RefManager();

  template <class C> inline persistent<C> new_persistent(C *obj);

  abstract_ref_block *top() { return top_; }
  void set_top(abstract_ref_block *value) { top_ = value; }

private:
  friend class ref_iterator;
  friend class persistent_iterator;
  template <class C> friend class persistent;

  void dispose(persistent_cell &cell);

  abstract_ref_block *top_;

  buffer<persistent_cell*> &persistent_list() { return persistent_list_; }
  buffer<persistent_cell*> persistent_list_;
};


class ref_iterator : public nocopy {
public:
  inline ref_iterator(RefManager &refs);
  inline bool has_next();
  inline Value *&next();
private:
  inline void advance();
  abstract_ref_block *current_;
  word next_index_;
};


class persistent_iterator : public nocopy {
public:
  inline persistent_iterator(RefManager &refs);
  inline bool has_next();
  inline Value *&next();
private:
  RefManager &refs() { return refs_; }
  RefManager &refs_;
  word index_;
};


// ---------------------------
// --- R e f e r e n c e s ---
// ---------------------------

/**
 * An abstract superclass used to hold the behavior shared between
 * instantiations of ref.
 */
template <class C>
class ref {
public:
  inline explicit ref(Value **cell = 0) : cell_(cell) { }
  template <class D> inline ref(ref<D> that)
      : cell_(that.cell()) { }
  inline C* operator*();
  inline C* operator->();
  inline Value** cell() { return cell_; }
  template <class D>
  bool operator==(ref<D> that) { return this->operator*() == *that; }
  inline bool is_empty() { return cell_ == 0; }
  static inline ref<C> empty();
protected:
  Value **cell_;
};

class persistent_cell : public nocopy {
public:
  persistent_cell(Value *value, RefManager &refs, word index)
    : cell_(value)
    , refs_(refs)
    , index_(index) { }
  Value **cell() { return &cell_; }
  word index() { return index_; }
  void set_index(word value) { index_ = value; }
  void dispose();
private:
  template <class C> friend class persistent;
  Value *cell_;
  RefManager &refs_;
  word index_;
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

#endif // _RUNTIME_REF
