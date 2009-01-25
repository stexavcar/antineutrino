#ifndef _UTILS_BUFFER
#define _UTILS_BUFFER

#include "utils/array.h"
#include "utils/global.h"
#include "utils/smart-ptrs.h"

namespace neutrino {

template < typename T, class A = new_delete_array_allocator<T> >
class buffer : public nocopy {
public:
  inline buffer(A allocator = A());
  inline ~buffer();
  void append(const T &obj);
  void push(const T &obj);
  T remove_last();
  T pop();
  const T &peek();
  T &operator[](word index);
  T *start() { return data_.start(); }
  word length() { return length_; }
  void ensure_capacity(word length);
  array<T> allocate(word length);
  array<T> raw_data();
  vector<T> as_vector();

private:
  void extend_capacity(word required);
  A allocator() { return allocator_; }
  static const word kInitialCapacity = 4;
  array<T> data_;
  word length_;
  word capacity_;
  A allocator_;
};

template <typename T, class D = ptr_delete<T> >
class own_buffer : public buffer<T*> {
public:
  inline own_buffer() : buffer<T*>() { }
  ~own_buffer();
};

}

#endif // _UTILS_BUFFER
