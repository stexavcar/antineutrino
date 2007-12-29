#ifndef _UTILS_CONDITIONS
#define _UTILS_CONDITIONS

#include "utils/types.h"

namespace neutrino {

#define FOR_EACH_CONDITION(VISIT)                                    \
  VISIT(UNKNOWN)         VISIT(OUT_OF_BOUNDS)   VISIT(NO_REF_SCOPE)  \
  VISIT(CAST_ERROR)      VISIT(VALIDATION)      VISIT(DISALLOWED)

enum Condition {
  NO_PROBLEM = 0
#define DECLARE_ENUM(NAME) , NAME
FOR_EACH_CONDITION(DECLARE_ENUM)
#undef DECLARE_ENUM
};

/**
 * The conditions are functions that can be called when something
 * unexpected or problematic occurs.  When this happens the condition
 * will print an error message, and bail out in a suitable way.
 */
class Conditions {
public:
  virtual ~Conditions() { }
  
  void check_failed(const char *file_name, int line_number,
      const char *source, bool value, Condition cause);

  void check_eq_failed(const char* file_name, int line_number,
      int expected, const char *expected_source, int value,
      const char *value_source, Condition cause);

  void check_ge_failed(const char* file_name, int line_number,
      int32_t value, const char *value_source, int32_t limit,
      const char *limit_source, Condition cause);

  void check_lt_failed(const char* file_name, int line_number,
      int value, const char *value_source, int limit,
      const char *limit_source, Condition cause);

  void check_eq_failed(const char* file_name, int line_number,
      Value *expected, const char *expected_source,
      Value *value, const char *value_source, Condition cause);

  void check_is_failed(const char *file_name, int line_number,
      const char *type_name, uint32_t type_tag, Data *value,
      const char *value_source, Condition cause);
  
  void unreachable(const char *file_name, int line_number,
      Condition cause);

  void unhandled(const char *file_name, int line_number,
      const char *enum_name, int32_t value, AbstractEnumInfo &info,
      Condition cause);
  
  void error_occurred(const char *format, ...);
  
  virtual void notify(Condition cause);
  
  void abort();
  
  static Conditions &get();
  static void set(Conditions *that) { current_ = that; }

private:
  static Conditions *current_;
};

}

#endif // _UTILS_CONDITIONS
