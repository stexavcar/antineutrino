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

class Checks {
public:

  static inline void check(string file_name, int line_number,
      string source, bool value, Condition cause = cnUnknown) {
    if (!value) {
      Conditions::get().check_failed(file_name, line_number,
          source, value, cause);
    }
  }
  
  static inline void check_eq(string file_name, int line_number,
      int expected, string expected_source, int value,
      string value_source, Condition cause = cnUnknown) {
    if (expected != value) {
      Conditions::get().check_eq_failed(file_name, line_number,
          expected, expected_source, value, value_source, cause);
    }
  }
  
  static inline void check_eq(string file_name, int line_number,
      string expected, string expected_source, string value,
      string value_source, Condition cause = cnUnknown) {
    if (expected != value) {
      Conditions::get().check_eq_failed(file_name, line_number,
          expected, expected_source, value, value_source, cause);
    }
  }

  static inline void check_ge(string file_name, int line_number,
      word value, string value_source, word limit,
      string limit_source, Condition cause = cnUnknown) {
    if (value < limit) {
      Conditions::get().check_ge_failed(file_name, line_number,
          value, value_source, limit, limit_source, cause);
    }
  }
  
  static inline void check_lt(string file_name, int line_number,
      int value, string value_source, int limit,
      string limit_source, Condition cause = cnUnknown) {
    if (value >= limit) {
      Conditions::get().check_lt_failed(file_name, line_number,
          value, value_source, limit, limit_source, cause);
    }
  }
    
  static inline void check_eq(string file_name, int line_number,
      Value *expected, string expected_source, Value *value,
      string value_source, Condition cause = cnUnknown) {
    if (!expected->equals(value)) {
      Conditions::get().check_eq_failed(file_name, line_number,
          expected, expected_source, value, value_source, cause);
    }
  }

  static inline void check_is(string file_name, int line_number,
      string type_name, uword type_tag, Data *data,
      string value_source, bool holds, Condition cause = cnUnknown) {
    if (!holds) {
      Conditions::get().check_is_failed(file_name, line_number,
          type_name, type_tag, data, value_source, cause);
    }
  }
  
};

// --- C h e c k   M a c r o s ---

#define CHECK(value) neutrino::Checks::check(__FILE__, __LINE__,     \
    #value, value)

#define CHECK_C(COND, value) neutrino::Checks::check(__FILE__,       \
    __LINE__, #value, value, COND)

#define CHECK_EQ(expected, value) neutrino::Checks::check_eq(        \
    __FILE__, __LINE__, expected, #expected, value, #value)

#define CHECK_EQ_C(COND, expected, value) neutrino::Checks::check_eq(\
    __FILE__, __LINE__, expected, #expected, value, #value, COND)

#define CHECK_GE(value, limit) neutrino::Checks::check_ge(           \
    __FILE__, __LINE__, value, #value, limit, #limit)

#define CHECK_LT(value, limit) neutrino::Checks::check_lt(           \
    __FILE__, __LINE__, value, #value, limit, #limit)

#define CHECK_LT_C(COND, value, limit) neutrino::Checks::check_lt(   \
    __FILE__, __LINE__, value, #value, limit, #limit, COND)

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
