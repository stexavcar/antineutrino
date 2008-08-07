#ifndef _UTILS_MISC
#define _UTILS_MISC

namespace neutrino {

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
