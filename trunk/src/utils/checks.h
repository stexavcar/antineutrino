#ifndef _UTILS_CHECK
#define _UTILS_CHECK

#include "heap/values.h"
#include "utils/conditions.h"
#include "utils/globals.h"

namespace neutrino {


// -----------------------------------
// --- D y n a m i c   C h e c k s ---
// -----------------------------------

class AbstractEnumInfo {
public:
  virtual ~AbstractEnumInfo() { }
  virtual string get_name_for(int32_t) = 0;
};

class Checks {
public:

  static inline void check(const char *file_name, int line_number,
      const char *source, bool value, Condition cause = UNKNOWN) {
    if (!value) {
      Conditions::get().check_failed(file_name, line_number,
          source, value, cause);
    }
  }
  
  static inline void check_eq(const char* file_name, int line_number,
      int expected, const char *expected_source, int value,
      const char *value_source, Condition cause = UNKNOWN) {
    if (expected != value) {
      Conditions::get().check_eq_failed(file_name, line_number,
          expected, expected_source, value, value_source, cause);
    }
  }
  
  static inline void check_lt(const char* file_name, int line_number,
      int value, const char *value_source, int limit,
      const char *limit_source, Condition cause = UNKNOWN) {
    if (value >= limit) {
      Conditions::get().check_lt_failed(file_name, line_number,
          value, value_source, limit, limit_source, cause);
    }
  }
    
  static inline void check_eq(const char* file_name, int line_number,
      Value *expected, const char *expected_source, Value *value,
      const char *value_source, Condition cause = UNKNOWN) {
    if (!expected->equals(value)) {
      Conditions::get().check_eq_failed(file_name, line_number,
          expected, expected_source, value, value_source, cause);
    }
  }

  static inline void check_is(const char *file_name, int line_number,
      const char *type_name, uint32_t type_tag, Data *data,
      const char *value_source, bool holds, Condition cause = UNKNOWN) {
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

#define CHECK_LT(value, limit) neutrino::Checks::check_lt(        \
    __FILE__, __LINE__, value, #value, limit, #limit)

#define CHECK_IS(Type, value)                                        \
    neutrino::Checks::check_is(__FILE__, __LINE__, #Type,            \
        ValueInfo<Type>::kTag, value, #value, is<Type>(value))

#define CHECK_IS_C(COND, Type, value)                                \
    neutrino::Checks::check_is(__FILE__, __LINE__, #Type,            \
        ValueInfo<Type>::kTag, value, #value, is<Type>(value), COND)

#define UNREACHABLE() neutrino::Conditions::get().unreachable(       \
    __FILE__, __LINE__, neutrino::UNKNOWN)

// --- E n u m   a s s e r t i o n s ---

#ifdef DEBUG

#define GC_SAFE_CHECK_IS_C(COND, Type, value)                        \
    neutrino::Checks::check_is(__FILE__, __LINE__, #Type,            \
        ValueInfo<Type>::kTag, value, #value,                        \
        gc_safe_is<Type>(value), COND)

#define UNHANDLED(Enum, value) do {                                  \
    neutrino::EnumInfo<Enum> enum_info;                              \
    neutrino::Conditions::get().unhandled(__FILE__, __LINE__, #Enum, \
        (value), enum_info, neutrino::UNKNOWN);                      \
  } while (false)

/**
 * Using these macros you can define a mapping from enum values to
 * string names for use with the UNHANDLED macro.
 */

#define MAKE_ENUM_INFO_HEADER(Enum)                                  \
  template <> class EnumInfo<Enum> : public AbstractEnumInfo {       \
  public:                                                            \
    virtual string get_name_for(int32_t kind) {                      \
      switch (kind) {

#define MAKE_ENUM_INFO_FOOTER()                                      \
        default: return "<unknown>";                                 \
      }                                                              \
    }                                                                \
  };

#define MAKE_ENUM_INFO_ENTRY(NAME) case NAME: return #NAME;

template <typename Enum>
class EnumInfo : public AbstractEnumInfo {
public:
  virtual string get_name_for(int32_t);
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

#define ASSERT(value)                  IF_DEBUG(CHECK(value))
#define ASSERT_C(cond, value)          IF_DEBUG(CHECK_C(cond, value))
#define ASSERT_EQ(expected, value)     IF_DEBUG(CHECK_EQ(expected, value))
#define ASSERT_LT(expected, value)     IF_DEBUG(CHECK_LT(expected, value))
#define ASSERT_IS(Type, value)         IF_DEBUG(CHECK_IS(Type, value))
#define ASSERT_IS_C(cond, Type, value) IF_DEBUG(CHECK_IS_C(cond, Type, value))


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
    typedef StaticAssertionHelper<sizeof(StaticAssertion<static_cast<bool>(test)>)> \
      SEMI_STATIC_JOIN(__StaticAssertTypedef__, __LINE__)

} // namespace neutrino

#endif // _UTILS_CHECK
