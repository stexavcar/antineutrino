#ifndef COMPILER_COMPILE_UTILS
#define COMPILER_COMPILE_UTILS

#include "heap/ref-inl.h"

namespace neutrino {

class heap_list {
public:
  inline heap_list(Runtime &runtime);
  likely initialize();
  ~heap_list();
  uword length() { return length_; }
  inline Value *operator[](uword index);
  inline Value *get(uword index);
  likely append(ref<Value> value);
  likely extend_capacity();
private:
  Runtime &runtime() { return runtime_; }
  persistent<Tuple> data() { return data_; }
  void set_data(persistent<Tuple> value) { data_ = value; }
  Runtime &runtime_;
  persistent<Tuple> data_;
  uword length_;
};

}

#endif // COMPILER_COMPILE_UTILS
