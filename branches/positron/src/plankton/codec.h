#ifndef _PLANKTON_CODEC
#define _PLANKTON_CODEC

#include "utils/global.h"

namespace positron {

template <typename Out>
class Encoder {
public:
  static void encode_uint32(uint32_t value, Out &out);
};

template <typename In>
class Decoder {
public:
  static uint32_t decode_uint32(In &in);
};

class Codec {
public:
  static const word kMaxEncodedInt32Size = 4;
};

} // namespace positron

#endif // _PLANKTON_CODEC
