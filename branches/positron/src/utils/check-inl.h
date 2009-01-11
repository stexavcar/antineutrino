#ifndef _UTILS_CHECKS_INL
#define _UTILS_CHECKS_INL

#include "utils/check.h"
#include "platform/abort.h"
#include "utils/string-inl.h"

namespace positron {

template <class P, typename T>
void Check::predicate(const T &a, const T &b, const char *a_src,
    const char *b_src) {
  if (!P::compare(a, b)) {
    predicate_failed(dHere, P::format(), P::details(), args(a, b, string(a_src), string(b_src)));
  }
}

} // namespace positron


#endif // _UTILS_CHECKS_INL
