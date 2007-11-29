#ifndef COMPILER_COMPILE_UTILS
#define COMPILER_COMPILE_UTILS

#include "heap/ref-inl.h"

namespace neutrino {

class heap_list {
public:
  inline heap_list(Factory &factory);
  void initialize();
  ~heap_list();
  uint32_t length() { return length_; }
  inline ref<Value> operator[](uint32_t index);
  inline Value *get(uint32_t index);
  void append(ref<Value> value);
  void extend_capacity();
private:
  Factory &factory() { return factory_; }
  persistent<Tuple> data() { return data_; }
  void set_data(persistent<Tuple> value) { data_ = value; }
  Factory &factory_;
  persistent<Tuple> data_;
  uint32_t length_;
};

}

#endif // COMPILER_COMPILE_UTILS
