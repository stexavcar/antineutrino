#ifndef _UTILS_CONDITIONS
#define _UTILS_CONDITIONS

#include "utils/string.h"
#include "utils/types.h"

namespace neutrino {


#define eConditions(VISIT)                                           \
  VISIT(Unknown)         VISIT(OutOfBounds)   VISIT(NoRefScope)      \
  VISIT(CastError)       VISIT(Validation)    VISIT(Disallowed)      \
  VISIT(ForwarderState)  VISIT(RefOverflow)


enum Condition {
  cnNoProblem = 0
#define DECLARE_ENUM(Name) , cn##Name
eConditions(DECLARE_ENUM)
#undef DECLARE_ENUM
};


class Log {
public:

  void info(string format, const var_args &args);
  static Log &get() { return instance_; }

private:
  static Log instance_;
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

  void check_predicate_failed(string file_name, int line_number,
      const variant &expected, string expected_source,
      const variant &value, string value_source,
      string name, Condition cause);

  void check_is_failed(string file_name, int line_number,
      string type_name, uword type_tag, Data *value,
      string value_source, Condition cause);

  void unreachable(string file_name, int line_number,
      Condition cause);

  void unhandled(string file_name, int line_number,
      string enum_name, word value, AbstractEnumInfo &info,
      Condition cause);

  void error_occurred(string format, const var_args &args);
  virtual void notify(Condition cause);
  void abort(string message);

  static Conditions &get();
  static void set(Conditions *that) { current_ = that; }

private:
  static Conditions *current_;
};

}

#endif // _UTILS_CONDITIONS
