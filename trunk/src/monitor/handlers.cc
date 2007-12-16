#include "monitor/handlers.h"
#include "monitor/monitor.h"

namespace neutrino {

RequestHandler Handlers::get_handler(string name) {
#define MAKE_CASE(handler_name, handler_func)                        \
  if (name == #handler_name) return handler_func;
FOR_EACH_MONITOR_HANDLER(MAKE_CASE)
#undef MAKE_CASE
  return NULL;
}

string Handlers::get_variable_value(string name) {
  // 'all' returns a record with the value of all variables
  if (name == "all") {
    string_buffer buf;
    buf.append("({");
    MonitoredVariable *current = MonitoredVariable::first();
    bool is_first = true;
    while (current != NULL) {
      if (is_first) is_first = false;
      else buf.append(", ");
      buf.printf("'%': %", current->name().chars(), current->value());
      current = current->next();
    }
    buf.append("})");
    return buf.to_string();
  } else {
    MonitoredVariable *current = MonitoredVariable::first();
    while (current != NULL) {
      if (current->name() == name) {
        string_buffer buf;
        buf.printf("%", current->value());
        return buf.to_string();
      }
      current = current->next();
    }
  }
  return string();
}

} // neutrino
