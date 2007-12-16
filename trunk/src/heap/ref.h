#ifndef _HANDLE_
#define _HANDLE_

#include "utils/list.h"
#include "utils/types.h"
#include "monitor/monitor.h"

namespace neutrino {

// --- R e f   S c o p e ---

class RefBlock {
public:
  Value **first_cell() { return &entries_[0]; }
  Value **limit() { return &entries_[kSize]; }
private:
  static const int kSize = 256;
  Value *entries_[kSize];
};

class RefScopeInfo {
public:
  inline RefScopeInfo();
  int32_t block_count;
  Value **next_cell;
  Value **limit;
};

/**
 * A scope for allocating new refs.
 */
class RefScope {
public:
  inline RefScope();
  inline ~RefScope();
  
  template <class C>
  static inline C **new_cell(C *value);
private:
  static void shrink();
  static Value **grow();
  
  DECLARE_MONITOR_VARIABLE(count);
  DECLARE_MONITOR_VARIABLE(high_water_mark);

  static RefScopeInfo &current() { return current_; }
  static RefScopeInfo current_;
  const RefScopeInfo previous_;
  
  friend class RefIterator;
  static list_buffer<RefBlock*> &block_stack() { return block_stack_; }
  static list_buffer<RefBlock*> block_stack_;
  static RefBlock *spare_block() { return spare_block_; }
  static RefBlock *spare_block_;
};

class RefIterator {
public:
  inline RefIterator();
  inline bool has_next();
  inline Value *&next();
private:
  uint32_t current_block_;
  Value **current_, **limit_;
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

template <class C>
class persistent : public ref<C> {
public:
  inline persistent(C **cell) : ref<C>(cell) { }
  inline persistent() : ref<C>() { }
  inline void dispose() { delete this->cell(); }
};

template <class C> static inline ref<C> new_ref(C *obj);
template <class C> static inline persistent<C> new_persistent(C *obj);

}

#endif // _HANDLE_
