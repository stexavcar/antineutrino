#ifndef _UTILS_MIRROR
#define _UTILS_MIRROR

#include "heap/values.h"

namespace neutrino {

class Mirror {
public:
  Mirror() : type_(__first_instance_type), string_(0), number_(0) { }
  InstanceType type_;
  char *string_;
  double number_;
};

} // neutrino

#endif // _UTILS_MIRROR
