#ifndef _UTILS_CHECK
#define _UTILS_CHECK

#include "utils/string.h"

namespace positron {

class Eq {
public:
  template <typename T>
  static inline bool compare(const T &a, const T &b) { return a == b; }
  static inline string format() { return "@2 == @3"; }
  static inline string details() { return "Found: @0{q}\nExpected: @1{q}"; }
};

class Lt {
public:
  template <typename T>
  static inline bool compare(const T &a, const T &b) { return a < b; }
  static inline string format() { return "@2 < @3"; }
  static inline string details() { return string(); }
};

class Leq {
public:
  template <typename T>
  static inline bool compare(const T &a, const T &b) { return a <= b; }
  static inline string format() { return "@2 <= @3"; }
  static inline string details() { return string(); }
};

class Check {
public:
  template <class P, typename T>
  static inline void predicate(const T &a, const T &b, const char *a_src,
      const char *b_src);
  static void predicate_failed(SourceLocation here, string format,
      string details, const var_args &args);
};

#define cCheckPred(expected, Pred, value) positron::Check::predicate<Pred>(expected, value, #expected, #value)

#define cCheckEq(left, right) cCheckPred(left, positron::Eq, right)
#define cCheckLt(left, right) cCheckPred(left, positron::Lt, right)
#define cCheckLeq(left, right) cCheckPred(left, positron::Leq, right)


} // namespace positron

#endif // _UTILS_CHECK
