#ifndef _UTILS_GLOBAL
#define _UTILS_GLOBAL

#include <cstdio>
#include <cstdlib>
#include <new>

#include "utils/pollock.h"
#include "utils/types.h"

namespace neutrino {


typedef signed long word;
typedef unsigned long uword;
typedef uint32_t code_point;


template <typename T> struct coerce { typedef const T &type; };
template <> struct coerce<const char*> { typedef string type; };
template <> struct coerce<char*> { typedef string type; };

template <typename T>
class wrapper_t {
public:
  explicit wrapper_t(const T &value) : value_(value) { }
  const T &operator*() { return value_; }
private:
  T value_;
};

template <> struct coerce<bool> { typedef wrapper_t<bool> type; };
template <> struct coerce<char> { typedef wrapper_t<char> type; };
template <> struct coerce<double> { typedef wrapper_t<double> type; };

#define TYPE_CHECK(S, T)                                             \
  do {                                                               \
    if (false)                                                       \
      *static_cast<T**>(NULL) = static_cast<S*>(NULL);               \
  } while (false)


namespace p = plankton;


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

static const word kWordSize = sizeof(word);
static const word kPointerSize = sizeof(void*);

/**
 * Returns the new extended capacity given that the current capacity
 * is the given value.  The result approximates (to within 1%) an
 * increase in size by the golden ratio but is computed purely with
 * simple integer operations.
 */
static inline word grow_value(word n) {
  return ((n << 3) + (n << 2) + n) >> 3;
}

static inline word round_to_power_of_two(word value, word boundary) {
  return value + (-value & (boundary - 1));
}

static inline uint64_t double_bits(double value) {
  return *reinterpret_cast<uint64_t*>(&value);
}

static inline double make_double(uint64_t value) {
  return *reinterpret_cast<double*>(&value);
}

#define KB * 1024
#define MB * 1024 KB

class SourceLocation {
public:
  SourceLocation(const char *file, word line)
    : file_(file), line_(line) { }
  // Uses const char* rather than string to not get a dependency on
  // string.h from global.h.
  const char *file() const { return file_; }
  const char *short_name() const;
  word line() const { return line_; }
private:
  const char *file_;
  word line_;
};

#define dHere neutrino::SourceLocation(__FILE__, __LINE__)

#ifdef DEBUG
#define ARRAY_BOUNDS_CHECKS
#define ccArrayBoundsChecks(then_part, else_part) then_part
#define ccDebug(then_part, else_part) then_part
#else
#define ccDebug(then_part, else_part) else_part
#define ccArrayBoundsChecks(then_part, else_part) else_part
#endif

#ifdef M32
#define cc32(then_part, else_part) then_part
#define cc64(then_part, else_part) else_part
#else
#define cc32(then_part, else_part) else_part
#define cc64(then_part, else_part) then_part
#endif

#ifdef MSVC
#define ccWin32(then_part, else_part) then_part
#else
#define ccWin32(then_part, else_part) else_part
#endif

#define SEMI_STATIC_JOIN(a, b) SEMI_STATIC_JOIN_HELPER(a, b)
#define SEMI_STATIC_JOIN_HELPER(a, b) a##b

#define IF(ccCond, then_part) ccCond(then_part, typedef void SEMI_STATIC_JOIN(__If__, __LINE__))
#define IF_ELSE(ccCond, then_part, else_part) ccCond(then_part, else_part)

} // namespace neutrino

#endif // _UTILS_GLOBAL