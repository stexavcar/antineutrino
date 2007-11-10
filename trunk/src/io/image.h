#ifndef _IO_IMAGE
#define _IO_IMAGE

#include "heap/roots.h"
#include "utils/consts.h"
#include "utils/globals.h"
#include "utils/types.h"

namespace neutrino {

#define DECLARE_IMAGE_OBJECT_CONST(n, Type, Const)                   \
  static const uint32_t Image##Type##_##Const = n;
FOR_EACH_IMAGE_OBJECT_CONST(DECLARE_IMAGE_OBJECT_CONST)
#undef DECLARE_IMAGE_OBJECT_CONST

class ImageData {
public:
  static inline ImageData *from(uint32_t value);
};

class ImageForwardPointer : public ImageData {
public:
  inline Object *target();
  static inline ImageForwardPointer *to(Object *obj);
};

class ImageValue : public ImageData {
public:
};

class ImageSmi : public ImageValue {
public:
  inline int32_t value();
};

class ImageObject : public ImageValue {
public:
  uint32_t instance_type();
  void point_forward(Object *target);
  uint32_t memory_size();

};

class ImageString : public ImageObject {
public:
  inline uint32_t length();
  uint32_t string_memory_size();
};

class ImageTuple : public ImageObject {
public:
  inline uint32_t length();
  uint32_t tuple_memory_size();
};

class ImageCode : public ImageObject {
public:
  inline uint32_t length();
  uint32_t code_memory_size();
};

class ImageDictionary : public ImageObject {
public:
  
};

template <class C>
static inline bool is(ImageData *val);

template <class C>
static inline C *image_cast(ImageData *val);

class Image {
public:
  Image(uint32_t size, uint32_t *data);
  bool initialize();
  bool load();
  static inline Image &current();
  uint32_t *heap() { return heap_; }

  class Scope {
  public:
    Scope(Image &image);
    ~Scope();
  private:
    Image *previous_;
  };

private:
  typedef void (ObjectCallback)(ImageObject *obj);
  void for_each_object(ObjectCallback callback);
  static void copy_shallow(ImageObject *obj);
  uint32_t heap_size() { return heap_size_; }
  
  uint32_t size_, heap_size_;
  uint32_t *data_, *heap_;
  ImageValue *roots_[Roots::kCount];
  static Image *current_;
  
  static const uint32_t kMagicNumber = 0xFABACEAE;
  static const uint32_t kMagicNumberOffset = 0;
  static const uint32_t kHeapSizeOffset    = kMagicNumberOffset + 1;
  static const uint32_t kRootCountOffset   = kHeapSizeOffset + 1;
  static const uint32_t kRootsOffset       = kRootCountOffset + 1;
  static const uint32_t kHeaderSize        = kRootsOffset + Roots::kCount;

};

}

#endif // _IO_IMAGE
