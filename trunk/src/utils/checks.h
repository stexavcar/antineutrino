#ifndef _UTILS_CHECK
#define _UTILS_CHECK

#include "utils/conditions.h"
#include "utils/globals.h"
#include "values/values.h"

namespace neutrino {


// -----------------------------------
// --- D y n a m i c   C h e c k s ---
// -----------------------------------

class AbstractEnumInfo {
public:
  virtual ~AbstractEnumInfo() { }
  virtual string get_name_for(word) = 0;
};

template <typename T>
class CheckComparer {
public:
  static inline bool compare(T a, T b) { return a == b; }
};

class Checks {
public:
  
  struct Equals {
    template <typename T>
    static inline bool compare(const T &a, const T &b) { return CheckComparer<T>::compare(a, b); }
    static inline string name() { return "=="; }
  };

  struct LessThan {
    template <typename T>
    static inline bool compare(const T &a, const T &b) { return a < b; }
    static inline string name() { return "<"; }
  };

  struct LessOrEquals {
    template <typename T>
    static inline bool compare(const T &a, const T &b) { return a <= b; }
    static inline string name() { return "<="; }
  };

  struct GreaterOrEquals {
    template <typename T>
    static inline bool compare(const T &a, const T &b) { return a >= b; }
    static inline string name() { return ">="; }
  };

  // We use c strings for the arguments that correspond to the
  // "macro" strings, like file name and source code.  This is to
  // avoid the overhead in debug mode of calculating the length of
  // the strings whenever there is an assertion, since debug mode
  // doesn't optimize strlen of literal strings.  This should be protect
  // since there are no null characters in file names of source code.
  
  static inline void check(const char *file_name, int line_number,
      const char *source, bool value, Condition cause = cnUnknown) {
    if (!value) {
      Conditions::get().check_failed(file_name, line_number,
          source, value, cause);
    }
  }
  
  template <class Comparer, typename T>
  static inline void check_predicate(const char *file_name,
      int line_number, const T &expected, const char *expected_source,
      const T &value, const char *value_source,
      Condition cause = cnUnknown) {
    if (!Comparer::compare(expected, value)) {
      Conditions::get().check_predicate_failed(file_name, line_number,
          expected, expected_source, value, value_source,
          Comparer::name(), cause);
    }
  }
  
  static inline void check_is(const char *file_name, int line_number,
      const char *type_name, uword type_tag, Data *data,
      const char *value_source, bool holds, Condition cause = cnUnknown) {
    if (!holds) {
      Conditions::get().check_is_failed(file_name, line_number,
          type_name, type_tag, data, value_source, cause);
    }
  }

};


// This helper class exists to make the template and inference magic
// add up.
template <class Predicate>
class PredicateCheckHelper {
public:
  // The generic assertion method that checks its arguments according
  // to the Predicate.
  template <typename E, typename A>
  static inline void check(const char *file_name, int line_number,
      const E &expected, const char *expected_source, const A &actual,
      const char *actual_source, Condition cause = cnUnknown) {
    check_helper<A>(file_name, line_number, expected, expected_source,
        actual, actual_source, cause);
  }
private:
  // Utility function whose function is to turn the two type arguments
  // E and A to 'check' into just one, T.  'check' uses this to coerce
  // the expected value to the type of the actual value.
  template <typename T>
  static inline void check_helper(const char *file_name, int line_number,
      const T &expected, const char *expected_source, const T &actual,
      const char *actual_source, Condition cause) {
    if (!Predicate::compare(expected, actual)) {
      Conditions::get().check_predicate_failed(file_name, line_number,
          expected, expected_source, actual, actual_source,
          Predicate::name(), cause);
    }
  }
};

// --- C h e c k   M a c r o s ---

#define CHECK_PREDICATE(Predicate, expected, value)                  \
  neutrino::PredicateCheckHelper<neutrino::Checks::Predicate>::check(__FILE__,\
      __LINE__, expected, #expected, value, #value)

#define CHECK_PREDICATE_C(COND, Predicate, expected, value)          \
  neutrino::PredicateCheckHelper<neutrino::Checks::Predicate>::check(__FILE__,         \
      __LINE__, expected, #expected, value, #value, COND)

#define CHECK(value) neutrino::Checks::check(__FILE__, __LINE__,     \
    #value, value)

#define CHECK_C(COND, value) neutrino::Checks::check(__FILE__,       \
    __LINE__, #value, value, COND)

#define CHECK_EQ(expected, value) CHECK_PREDICATE(Equals, expected, value)
#define CHECK_EQ_C(COND, expected, value) CHECK_PREDICATE_C(COND, Equals, expected, value)

#define CHECK_GEQ(value, limit) CHECK_PREDICATE(GreaterOrEquals, value, limit)

#define CHECK_LT(value, limit) CHECK_PREDICATE(LessThan, value, limit)
#define CHECK_LT_C(COND, expected, value) CHECK_PREDICATE_C(COND, LessThan, expected, value)

#define CHECK_LEQ(value, limit) CHECK_PREDICATE(LessOrEquals, value, limit)

#define CHECK_IS(Type, value)                                        \
    neutrino::Checks::check_is(__FILE__, __LINE__, #Type,            \
        ValueInfo<Type>::kTag, value, #value, is<Type>(value))

#define CHECK_IS_C(COND, Type, value)                                \
    neutrino::Checks::check_is(__FILE__, __LINE__, #Type,            \
        ValueInfo<Type>::kTag, value, #value, is<Type>(value), COND)

#define UNREACHABLE() neutrino::Conditions::get().unreachable(       \
    __FILE__, __LINE__, neutrino::cnUnknown)

// --- E n u m   a s s e r t i o n s ---

#ifdef DEBUG

#define GC_SAFE_CHECK_IS_C(COND, Type, value)                        \
    neutrino::Checks::check_is(__FILE__, __LINE__, #Type,            \
        ValueInfo<Type>::kTag, value, #value,                        \
        gc_safe_is<Type>(value), COND)

#define UNHANDLED(Enum, value) do {                                  \
    neutrino::EnumInfo<Enum> enum_info;                              \
    neutrino::Conditions::get().unhandled(__FILE__, __LINE__, #Enum, \
        (value), enum_info, neutrino::cnUnknown);                    \
  } while (false)

/**
 * Using these macros you can define a mapping from enum values to
 * string names for use with the UNHANDLED macro.
 */

#define MAKE_ENUM_INFO_HEADER(Enum)                                  \
  template <>                                                        \
  string EnumInfo<Enum>::get_name_for(word kind) {                   \
      switch (kind) {

#define MAKE_ENUM_INFO_FOOTER()                                      \
      default: return "<unknown>";                                   \
    }                                                                \
  }

#define MAKE_ENUM_INFO_ENTRY(NAME) case NAME: return #NAME;

template <typename Enum>
class EnumInfo : public AbstractEnumInfo {
public:
  virtual string get_name_for(word);
};

#else // DEBUG

#define UNHANDLED(Type, value) UNREACHABLE()
#define MAKE_ENUM_INFO_HEADER(Enum)
#define MAKE_ENUM_INFO_FOOTER()
#define MAKE_ENUM_INFO_ENTRY(NAME)

#endif // DEBUG


// -------------------------------------------
// --- D y n a m i c   A s s e r t i o n s ---
// -------------------------------------------

#define ASSERT(value)                      IF_DEBUG(CHECK(value))
#define ASSERT_C(cond, value)              IF_DEBUG(CHECK_C(cond, value))
#define ASSERT_EQ(expected, value)         IF_DEBUG(CHECK_EQ(expected, value))
#define ASSERT_EQ_C(COND, expected, value) IF_DEBUG(CHECK_EQ_C(COND, expected, value))
#define ASSERT_GE(expected, value)         IF_DEBUG(CHECK_GE(expected, value))
#define ASSERT_LT(expected, value)         IF_DEBUG(CHECK_LT(expected, value))
#define ASSERT_LT_C(COND, expected, value) IF_DEBUG(CHECK_LT_C(COND, expected, value))
#define ASSERT_IS(Type, value)             IF_DEBUG(CHECK_IS(Type, value))
#define ASSERT_IS_C(COND, Type, value)     IF_DEBUG(CHECK_IS_C(COND, Type, value))


// -----------------------------------------
// --- S t a t i c   A s s e r t i o n s ---
// -----------------------------------------

// This is inspired by the static assertion facility in boost.  This
// is pretty magical.  If it causes you trouble on a platform you may
// find a fix in the boost code.
template <bool> class StaticAssertion;
template <> class StaticAssertion<true> { };

// Causes an error during compilation of the condition is not
// statically known to be true.  It is formulated as a typedef so that
// it can be used wherever a typedef can be used.  Beware that this
// actually causes each use to introduce a new defined type with a
// name depending on the source line.
template <int> class StaticAssertionHelper { };
#define STATIC_CHECK(test)                                           \
    typedef neutrino::StaticAssertionHelper<sizeof(neutrino::StaticAssertion<static_cast<bool>(test)>)> \
      SEMI_STATIC_JOIN(__StaticAssertTypedef__, __LINE__)

} // namespace neutrino

#endif // _UTILS_CHECK
