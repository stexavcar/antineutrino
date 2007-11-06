#ifndef _IO_IMAGE
#define _IO_IMAGE

#include "utils/globals.h"
#include "utils/types.h"

namespace neutrino {

class Image {
public:
  Image(uint32_t size, uint32_t *data);
  bool initialize();
private:
  static const uint32_t kMagicNumber = 0xFABACEAE;
  static const uint32_t kHeaderSize  = 2 * kWordSize;
  uint32_t size_, image_size_;
  uint32_t *data_;
  uint32_t *image_;
};

}

#endif // _IO_IMAGE
