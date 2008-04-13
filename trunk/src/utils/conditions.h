#ifndef _UTILS_CONDITIONS
#define _UTILS_CONDITIONS

#include "utils/string.h"
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
  
  void check_failed(string file_name, int line_number, string source,
      bool value, Condition cause);

  void check_eq_failed(string file_name, int line_number,
      int expected, string expected_source, int value,
      string value_source, Condition cause);

  void check_eq_failed(string file_name, int line_number,
      Value *expected, string expected_source,
      Value *value, string value_source, Condition cause);

  void check_eq_failed(string file_name, int line_number,
      string expected, string expected_source, string value,
      string value_source, Condition cause);
  
  void check_ge_failed(string file_name, int line_number,
      word value, string value_source, word limit,
      string limit_source, Condition cause);

  void check_lt_failed(string file_name, int line_number,
      int value, string value_source, int limit,
      string limit_source, Condition cause);

  void check_is_failed(string file_name, int line_number,
      string type_name, uword type_tag, Data *value,
      string value_source, Condition cause);
  
  void unreachable(string file_name, int line_number,
      Condition cause);

  void unhandled(string file_name, int line_number,
      string enum_name, word value, AbstractEnumInfo &info,
      Condition cause);
  
  void error_occurred(string format, list<fmt_elm> elms);
  void error_occurred(string format);
  void error_occurred(string format, fmt_elm elm1);
  void error_occurred(string format, fmt_elm elm1, fmt_elm elm2);
  void error_occurred(string format, fmt_elm elm1, fmt_elm elm2,
      fmt_elm elm3);
  
  virtual void notify(Condition cause);
  
  void abort(string message);
  
  static Conditions &get();
  static void set(Conditions *that) { current_ = that; }

private:
  static Conditions *current_;
};

}

#endif // _UTILS_CONDITIONS
