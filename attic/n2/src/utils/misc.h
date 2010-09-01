#ifndef _UTILS_MISC
#define _UTILS_MISC

namespace neutrino {

template <typename T>
static inline T min(const T &a, const T &b) {
  if (a < b) return a;
  else return b;
}

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

} // neutrino

#endif // _UTILS_MISC
