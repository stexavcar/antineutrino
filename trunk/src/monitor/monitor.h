#ifndef _MONITOR_MONITOR
#define _MONITOR_MONITOR

#include "utils/string.h"

namespace neutrino {

class MonitoredVariable {
public:
  explicit MonitoredVariable(string name, int32_t *variable);
  static MonitoredVariable *first() { return first_; }
  MonitoredVariable *next() { return next_; }
  string name() { return name_; }
  int32_t value() { return *variable_; }
private:
  string name_;
  int32_t *variable_;
  MonitoredVariable *next_;
  static MonitoredVariable *first_;
};

} // neutrino
 
#endif // _MONITOR_MONITOR
