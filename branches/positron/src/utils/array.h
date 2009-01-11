#ifndef _UTILS_ARRAY
#define _UTILS_ARRAY

namespace positron {

template <typename T>
class array {
public:
  array(T *data, int length) : data_(data), length_(length) { }
private:
  T *data_;
  int length_;
};

template <typename T, int L>
class embed_array {
public:
  T &operator[](int offset);
  const T &operator[](int offset) const;
private:
  T base_[L];
};

} // namespace positron

#endif // _UTILS_ARRAY
