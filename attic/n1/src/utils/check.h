#ifndef _UTILS_CHECK
#define _UTILS_CHECK

#include "utils/global.h"
#include "utils/string.h"

namespace neutrino {

class Eq {
public:
  template <typename S, typename T>
  static inline bool compare(const S &a, const T &b) { return a == b; }
  static inline string format() { return "@2 == @3"; }
  static inline string details() { return "Expected: @1{q}\nFound: @0{q}"; }
};

class Neq {
public:
  template <typename S, typename T>
  static inline bool compare(const S &a, const T &b) { return a != b; }
  static inline string format() { return "@2 != @3"; }
  static inline string details() { return "Value: @1{q}"; }
};

class Lt {
public:
  template <typename S, typename T>
  static inline bool compare(const S &a, const T &b) { return a < b; }
  static inline string format() { return "@2 < @3"; }
  static inline string details() { return "Left: @0{q}\nRight: @1{q}"; }
};

class Leq {
public:
  template <typename S, typename T>
  static inline bool compare(const S &a, const T &b) { return a <= b; }
  static inline string format() { return "@2 <= @3"; }
  static inline string details() { return "Left: @0{q}\nRight: @1{q}"; }
};

class Gt {
public:
  template <typename S, typename T>
  static inline bool compare(const S &a, const T &b) { return a > b; }
  static inline string format() { return "@2 > @3"; }
  static inline string details() { return "Left: @0{q}\nRight: @1{q}"; }
};

class Geq {
public:
  template <typename S, typename T>
  static inline bool compare(const S &a, const T &b) { return a >= b; }
  static inline string format() { return "@2 >= @3"; }
  static inline string details() { return "Left: @0{q}\nRight: @1{q}"; }
};

class Check {
public:
  template <class P, typename S, typename T>
  static inline void predicate(SourceLocation location, S a,
      T b, const char *a_src, const char *b_src);
  static void predicate_failed(SourceLocation here, string format,
      string details, const var_args &args);
  static inline void check(SourceLocation location, bool value,
      const char *src);
  static void check_failed(SourceLocation location, string source);
};

#define cCheckPred(expected, Pred, value) neutrino::Check::predicate<Pred>(dHere, expected, value, #expected, #value)
#define cCheck(expr) neutrino::Check::check(dHere, expr, #expr)

#define cCheckEq(left, right) cCheckPred(left, neutrino::Eq, right)
#define cCheckNeq(left, right) cCheckPred(left, neutrino::Neq, right)
#define cCheckLt(left, right) cCheckPred(left, neutrino::Lt, right)
#define cCheckLeq(left, right) cCheckPred(left, neutrino::Leq, right)
#define cCheckGt(left, right) cCheckPred(left, neutrino::Gt, right)
#define cCheckGeq(left, right) cCheckPred(left, neutrino::Geq, right)


} // namespace neutrino

#endif // _UTILS_CHECK