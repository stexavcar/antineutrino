#ifndef _MONITOR_MONITOR
#define _MONITOR_MONITOR

#include "utils/string.h"

namespace neutrino {

class MonitoredVariable {
public:
  explicit MonitoredVariable(string chlass, string name);
  int32_t get() { return value_; }
  void set(int32_t value) { value_ = value; }
  static MonitoredVariable *first() { return first_; }
  MonitoredVariable *next() { return next_; }
  string chlass() { return chlass_; }
  string name() { return name_; }
private:
  string chlass_;
  string name_;
  int32_t value_;
  MonitoredVariable *next_;
  static MonitoredVariable *first_;
};

#define DECLARE_MONITOR_VARIABLE(name) static MonitoredVariable &name
#define DEFINE_MONITOR_VARIABLE(chlass, name)                        \
  MonitoredVariable __##name##__(#chlass, #name);                    \
  MonitoredVariable &chlass::name = __##name##__

} // neutrino
 
#endif // _MONITOR_MONITOR
