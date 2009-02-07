#ifndef UTILS_STRING_INL
#define UTILS_STRING_INL

#include "utils/vector-inl.h"
#include "utils/buffer-inl.h"
#include "utils/string.h"

namespace neutrino {

template <word L>
const variant &var_args_impl<L>::operator[](word i) const {
  assert 0 <= i;
  assert i < L;
  return *elms_[i];
}


static inline var_args_impl<1> vargs(const variant &a0) {
  var_args_impl<1> result;
  result.elms_[0] = &a0;
  return result;
}


static inline var_args_impl<1> vargs() {
  return vargs(0);
}


static inline var_args_impl<2> vargs(const variant &a0, const variant &a1) {
  var_args_impl<2> result;
  result.elms_[0] = &a0;
  result.elms_[1] = &a1;
  return result;
}


static inline var_args_impl<3> vargs(const variant &a0, const variant &a1,
    const variant &a2) {
  var_args_impl<3> result;
  result.elms_[0] = &a0;
  result.elms_[1] = &a1;
  result.elms_[2] = &a2;
  return result;
}


static inline var_args_impl<4> vargs(const variant &a0, const variant &a1,
    const variant &a2, const variant &a3) {
  var_args_impl<4> result;
  result.elms_[0] = &a0;
  result.elms_[1] = &a1;
  result.elms_[2] = &a2;
  result.elms_[3] = &a3;
  return result;
}


} // namespace neutrino

#endif // _UTILS_STRING_INL
