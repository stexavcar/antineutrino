#ifndef _UTILS_GLOBAL
#define _UTILS_GLOBAL

#include <cstdlib>

#include "utils/pollock.h"

namespace positron {


typedef long word;


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
static inline size_t grow_value(size_t n) {
  return ((n << 3) + (n << 2) + n) >> 3;
}

class SourceLocation {
public:
  SourceLocation(const char *file, word line)
    : file_(file), line_(line) { }
  // Uses const char* rather than string to not get a dependency on
  // string.h from global.h.
  const char *file() const { return file_; }
  word line() const { return line_; }
private:
  const char *file_;
  word line_;
};

#define dHere positron::SourceLocation(__FILE__, __LINE__)

} // namespace positron

#endif // _UTILS_GLOBAL
