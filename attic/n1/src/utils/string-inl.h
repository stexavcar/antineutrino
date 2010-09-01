#ifndef UTILS_STRING_INL
#define UTILS_STRING_INL

#include "utils/vector-inl.h"
#include "utils/buffer-inl.h"
#include "utils/string.h"

namespace neutrino {


template <word L>
template <word T>
var_args_impl<L> &var_args_impl<L>::set(const variant &var) {
  elms_[T] = &var;
  return *this;
}


static inline var_args_impl<1> vargs(const variant &a0) {
  return var_args_impl<1>().set<0>(a0);
}


static inline var_args_impl<1> vargs() {
  return vargs(0);
}


static inline var_args_impl<2> vargs(const variant &a0, const variant &a1) {
  return var_args_impl<2>().set<0>(a0).set<1>(a1);
}


static inline var_args_impl<3> vargs(const variant &a0, const variant &a1,
    const variant &a2) {
  return var_args_impl<3>().set<0>(a0).set<1>(a1).set<2>(a2);
}


static inline var_args_impl<4> vargs(const variant &a0, const variant &a1,
    const variant &a2, const variant &a3) {
  return var_args_impl<4>().set<0>(a0).set<1>(a1).set<2>(a2).set<3>(a3);
}


} // namespace neutrino

#endif // _UTILS_STRING_INL
