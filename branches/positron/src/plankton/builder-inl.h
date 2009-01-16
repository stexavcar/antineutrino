#ifndef _PLANKTON_BUILDER_INL
#define _PLANKTON_BUILDER_INL

#include "plankton/builder.h"
#include "plankton/plankton.h"
#include "utils/array.h"
#include "utils/buffer.h"
#include "utils/string.h"

namespace positron {

void BuilderStream::write(uint8_t value) {
  builder().data().push(value);
}

} // namespace positron

#endif // _PLANKTON_BUILDER_INL
