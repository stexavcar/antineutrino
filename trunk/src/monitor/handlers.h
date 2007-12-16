#ifndef _MONITOR_HANDLERS
#define _MONITOR_HANDLERS

#include "utils/string.h"

namespace neutrino {

#define FOR_EACH_MONITOR_HANDLER(VISIT)                              \
  VISIT(vars, get_variable_value)

typedef string (*RequestHandler)(string);

class Handlers {
public:
  static RequestHandler get_handler(string name);
  static string get_variable_value(string variable);
};

}

#endif // _MONITOR_HANDLERS
