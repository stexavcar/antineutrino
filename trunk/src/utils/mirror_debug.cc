#include "utils/mirror.h"

#include "heap/values-inl.h"

namespace neutrino {

Mirror &Data::mirror() {
  Mirror &result = *(new Mirror());
  InstanceType type = gc_safe_type();
  result.type_ = type;
  switch (type) {
    case SMI_TYPE:
      result.number_ = cast<Smi>(this)->value();
      break;
    default:
      // do nuttin'
      break;
  }
  return result;
}

} // neutrino
