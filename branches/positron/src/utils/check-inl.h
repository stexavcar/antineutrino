#ifndef _UTILS_CHECKS_INL
#define _UTILS_CHECKS_INL

#include "utils/check.h"
#include "platform/abort.h"
#include "utils/string-inl.h"

namespace positron {

template <class P, typename S, typename T>
void Check::predicate(SourceLocation location, const S &a, const T &b,
    const char *a_src, const char *b_src) {
  if (!P::compare(a, b)) {
    predicate_failed(location, P::format(), P::details(), args(a, b,
        string(a_src), string(b_src)));
  }
}

} // namespace positron


#endif // _UTILS_CHECKS_INL
