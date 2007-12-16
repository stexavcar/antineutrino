#include "monitor/monitor.h"

namespace neutrino {

MonitoredVariable *MonitoredVariable::first_ = NULL;

MonitoredVariable::MonitoredVariable(string chlass, string name)
    : chlass_(chlass), name_(name), value_(0) {
  next_ = first_;
  first_ = this;
}

}
