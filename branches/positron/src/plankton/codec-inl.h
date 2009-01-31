#ifndef _PLANKTON_CODEC_INL
#define _PLANKTON_CODEC_INL

#include "plankton/codec.h"

namespace neutrino {

template <typename O>
void Base64Encoder<O>::add(uint8_t value) {
  buffer()[cursor_++] = value;
  if (cursor_ == kBufferSize) {
    flush();
    assert cursor_ == 0;
  }
}

static const char *kSixToEightMap = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

template <typename O>
void Base64Encoder<O>::flush() {
  if (cursor_ == 0) return;
  array<uint8_t> buf = buffer();
  out().add(kSixToEightMap[buf[0] >> 2]);
  switch (cursor_) {
    case 1:
      out().add(kSixToEightMap[((buf[0] & 0x03) << 4)]);
      out().add('=');
      out().add('=');
      break;
    case 2:
      out().add(kSixToEightMap[((buf[0] & 0x03) << 4) | (buf[1] >> 4)]);
      out().add(kSixToEightMap[((buf[1] & 0x0f) << 2)]);
      out().add('=');
      break;
    case 3:
      out().add(kSixToEightMap[((buf[0] & 0x03) << 4) | (buf[1] >> 4)]);
      out().add(kSixToEightMap[((buf[1] & 0x0f) << 2) | (buf[2] >> 6)]);
      out().add(kSixToEightMap[((buf[2] & 0x3f))]);
      break;
    default:
      assert false;
  }
  cursor_ = 0;
}

static const uint8_t kInvalidByte = 0xff;
static const uword kEightToSixMapSize = 80;
static const word kFirstEightToSixMapEntry = '+';
static const uint8_t kEightToSixMap[kEightToSixMapSize] = {
    62, kInvalidByte, kInvalidByte, kInvalidByte, 63, 52, 53, 54,
    55, 56, 57, 58, 59, 60, 61, kInvalidByte, kInvalidByte,
    kInvalidByte, kInvalidByte, kInvalidByte, kInvalidByte,
    kInvalidByte, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
    14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, kInvalidByte,
    kInvalidByte, kInvalidByte, kInvalidByte, kInvalidByte,
    kInvalidByte, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37,
    38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
};


template <typename O>
void Base64Decoder<O>::add(uint8_t value) {
  buffer()[cursor_++] = value;
  if (cursor_ == kBufferSize) {
    flush();
    assert cursor_ == 0;
  }
}

template <typename O>
bool Base64Decoder<O>::flush() {
  if (cursor_ == 0) return true;
  array<uint8_t> buf = buffer();
  embed_array<uint8_t, 4> tmp;
  word count;
  for (count = 0; (count < kBufferSize) && (buf[count] != '='); count++) {
    uword c = buf[count] - kFirstEightToSixMapEntry;
    if (c > kEightToSixMapSize)
      return false;
    if ((tmp[count] = kEightToSixMap[c]) == kInvalidByte)
      return false;
  }
  switch (count) {
    case 2:
      out().add((tmp[0] << 2) | (tmp[1] >> 4));
      break;
    case 3:
      out().add((tmp[0] << 2) | (tmp[1] >> 4));
      out().add((tmp[1] << 4) | (tmp[2] >> 2));
      break;
    case 4:
      out().add((tmp[0] << 2) | (tmp[1] >> 4));
      out().add((tmp[1] << 4) | (tmp[2] >> 2));
      out().add((tmp[2] << 6) | (tmp[3] >> 0));
      break;
    default:
      return false;
  }
  cursor_ = 0;
  return true;
}

} // namespace neutrino


#endif // _PLANKTON_CODEC_INL
