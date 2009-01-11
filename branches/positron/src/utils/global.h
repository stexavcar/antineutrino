#ifndef _UTILS_GLOBAL
#define _UTILS_GLOBAL

#include "utils/pollock.h"

namespace positron {

/**
 * Utility class that, if used as a base class, precludes instances
 * from being copied.  Inspired by (stolen from, really) boost.
 */
class nocopy {
protected:
  nocopy() { }
  ~nocopy() { }
private:
  nocopy(const nocopy&);
  const nocopy &operator=(const nocopy&);
};

/**
 * Returns the new extended capacity given that the current capacity
 * is the given value.  The result approximates (to within 1%) an
 * increase in size by the golden ratio but is computed purely with
 * simple integer operations.
 */
static inline int grow_value(int n) {
  return ((n << 3) + (n << 2) + n) >> 3;
}

class SourceLocation {
public:
  SourceLocation(const char *file, int line)
    : file_(file), line_(line) { }
  // Uses const char* rather than string to not get a dependency on
  // string.h from global.h.
  const char *file() const { return file_; }
  int line() const { return line_; }
private:
  const char *file_;
  int line_;
};

#define dHere SourceLocation(__FILE__, __LINE__)

} // namespace positron

#endif // _UTILS_GLOBAL
