#ifndef _PLANKTON_BUILDER
#define _PLANKTON_BUILDER

#include "plankton/plankton.h"
#include "utils/buffer.h"

namespace positron {

class Builder {
public:
  plankton::Integer new_integer(int32_t value);
private:
  word pos_;
  buffer<uint8_t> data_;
};

} // namespace positron

#endif // _PLANKTON_BUILDER
