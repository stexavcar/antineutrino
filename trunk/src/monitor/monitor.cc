#include "monitor/monitor.h"

namespace neutrino {

MonitoredVariable *MonitoredVariable::first_ = NULL;

MonitoredVariable::MonitoredVariable(string name, int32_t *variable)
    : name_(name), variable_(variable) {
  next_ = first_;
  first_ = this;
}

}
