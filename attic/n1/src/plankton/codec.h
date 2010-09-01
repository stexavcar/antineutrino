#ifndef _PLANKTON_CODEC
#define _PLANKTON_CODEC

#include "io/miniheap.h"
#include "plankton/plankton.h"
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


template <typename Out>
class Serializer {
public:
  Serializer(Out &out) : out_(out) { }
  void serialize(p::Value value);
private:
  void add_int32(int32_t value);
  Out &out() { return out_; }
  Out &out_;
};


template <typename In>
class Deserializer {
public:
  Deserializer(In &in) : in_(in) { }
  p::Value deserialize();
private:
  Factory &factory() { return factory_; }
  Factory factory_;
  int32_t get_int32();
  In &in() { return in_; }
  In &in_;
};


} // namespace neutrino

#endif // _PLANKTON_CODEC
