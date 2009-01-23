#ifndef _PLANKTON_CODEC_INL
#define _PLANKTON_CODEC_INL

#include "plankton/codec.h"

namespace neutrino {


template <typename Out>
void Encoder<Out>::encode_uint32(uint32_t value, Out &out) {
  out.write((value >> 0) & 0xFF);
  out.write((value >> 8) & 0xFF);
  out.write((value >> 16) & 0xFF);
  out.write((value >> 24) & 0xFF);
}


template <typename In>
uint32_t Decoder<In>::decode_uint32(In &in) {
  return (in.read() << 0)
      | (in.read() << 8)
      | (in.read() << 16)
      | (in.read() << 24);
}


} // namespace neutrino


#endif // _PLANKTON_CODEC_INL
