#ifndef _IO_IMAGE
#define _IO_IMAGE

#include "utils/consts.h"
#include "utils/globals.h"
#include "utils/types.h"

namespace neutrino {

#define DECLARE_IMAGE_OBJECT_FIELD_OFFSET(n, Type, Field)            \
  static const uint32_t Image##Type##_##Field##Offset = n;
FOR_EACH_IMAGE_OBJECT_FIELD(DECLARE_IMAGE_OBJECT_FIELD_OFFSET)
#undef DECLARE_IMAGE_OBJECT_FIELD_OFFSET

class ImageValue {
public:
  static inline ImageValue *from_addr(uint32_t addr);
};

class ImageSmi : public ImageValue {
  
};

class ImageObject : public ImageValue {
public:
  inline uint32_t type();
private:
  static const uint32_t kTypeOffset = ImageObject_TypeOffset;
};

template <class C>
static inline bool is(ImageValue *val);

class Image {
public:
  Image(uint32_t size, uint32_t *data);
  bool initialize();
private:
  static const uint32_t kMagicNumber = 0xFABACEAE;
  static const uint32_t kHeaderSize  = 2;
  uint32_t size_, image_size_;
  uint32_t *data_;
  uint32_t *image_;
};

}

#endif // _IO_IMAGE
