#ifndef _IO_IMAGE
#define _IO_IMAGE

#include "utils/globals.h"
#include "utils/types.h"

namespace neutrino {

class Image {
public:
  Image(uint32_t size, const uint8_t *data);
  bool reset();
  uint32_t read_word();
  bool has_more();
  Data **registers() { return registers_; }
private:
  static const uint32_t kMagicNumber = 0xFABACEAE;
  static const uint32_t kHeaderSize = 3 * kWordSize;
  uint32_t size_, cursor_, code_size_;
  const uint8_t *data_;
  Data **registers_;
};

}

#endif // _IO_IMAGE
