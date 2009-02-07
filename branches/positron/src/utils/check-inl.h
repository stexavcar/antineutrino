#ifndef _UTILS_CHECKS_INL
#define _UTILS_CHECKS_INL

#include "utils/check.h"
#include "platform/abort.h"
#include "utils/string-inl.h"

namespace neutrino {

template <class P, typename S, typename T>
void Check::predicate(SourceLocation location, S a, T b,
    const char *a_src, const char *b_src) {
  if (!P::compare(a, b)) {
    predicate_failed(location, P::format(), P::details(), vargs(a, b,
        string(a_src), string(b_src)));
  }
}

void Check::check(SourceLocation location, bool value, const char *src) {
  if (!value) {
    check_failed(location, src);
  }
}

} // namespace neutrino


#endif // _UTILS_CHECKS_INL
