#ifndef COMPILER_COMPILE_UTILS
#define COMPILER_COMPILE_UTILS

#include "heap/factory.h"

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
private:
  Factory &factory() { return factory_; }
  permanent<Tuple> data() { return data_; }
  Factory &factory_;
  permanent<Tuple> data_;
  uint32_t length_;
};

}

#endif // COMPILER_COMPILE_UTILS
