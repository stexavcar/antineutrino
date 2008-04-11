#ifndef _UTILS_CONDITIONS
#define _UTILS_CONDITIONS

#include "utils/types.h"

namespace neutrino {

#define eConditions(VISIT)                                           \
  VISIT(Unknown)         VISIT(OutOfBounds)   VISIT(NoRefScope)      \
  VISIT(CastError)       VISIT(Validation)    VISIT(Disallowed)      \
  VISIT(ForwarderState)

enum Condition {
  cnNoProblem = 0
#define DECLARE_ENUM(Name) , cn##Name
eConditions(DECLARE_ENUM)
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
      word value, const char *value_source, word limit,
      const char *limit_source, Condition cause);

  void check_lt_failed(const char* file_name, int line_number,
      int value, const char *value_source, int limit,
      const char *limit_source, Condition cause);

  void check_eq_failed(const char* file_name, int line_number,
      Value *expected, const char *expected_source,
      Value *value, const char *value_source, Condition cause);

  void check_is_failed(const char *file_name, int line_number,
      const char *type_name, uword type_tag, Data *value,
      const char *value_source, Condition cause);
  
  void unreachable(const char *file_name, int line_number,
      Condition cause);

  void unhandled(const char *file_name, int line_number,
      const char *enum_name, word value, AbstractEnumInfo &info,
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
