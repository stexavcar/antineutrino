#ifndef _UTILS_MIRROR
#define _UTILS_MIRROR

#include "heap/values.h"

namespace neutrino {

struct DataMirror {
  DataMirror(InstanceType type)
      : type_(type) { }
  InstanceType type_;
};

struct ClassMirror {
  ClassMirror(InstanceType instance_type)
      : instance_type_(instance_type) { }
  InstanceType instance_type_;
};

struct ObjectMirror {
  ObjectMirror(InstanceType type, Class *chlass)
    : type_(type)
    , chlass_(chlass) { }
  InstanceType type_;
  Class *chlass_;
};

struct StackMirror {
  StackMirror(uint32_t height, uint32_t fp, StackFlags flags)
      : height_(height)
      , fp_(fp)
      , flags_(flags) { }
  uint32_t height_, fp_;
  StackFlags flags_;
};

} // neutrino

#endif // _UTILS_MIRROR
