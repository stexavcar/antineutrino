#ifndef _PLANKTON_CODEC
#define _PLANKTON_CODEC

#include "utils/global.h"
#include "utils/array.h"
#include "utils/check.h"

namespace neutrino {

template <typename Out>
class Base64Encoder {
public:
  Base64Encoder(Out &out)
    : out_(out)
    , cursor_(0) { }
  void flush();
  void add(uint8_t v);
private:
  array<uint8_t> buffer() { return buffer_; }
  Out &out() { return out_; }
  static const word kBufferSize = 3;
  Out &out_;
  word cursor_;
  embed_array<uint8_t, kBufferSize> buffer_;
};

template <typename Out>
class Base64Decoder {
public:
  Base64Decoder(Out &out)
    : out_(out)
    , cursor_(0) { }
  bool flush();
  void add(uint8_t v);
private:
  array<uint8_t> buffer() { return buffer_; }
  Out &out() { return out_; }
  static const word kBufferSize = 4;
  Out &out_;
  word cursor_;
  embed_array<uint8_t, kBufferSize> buffer_;
};

} // namespace neutrino

#endif // _PLANKTON_CODEC
