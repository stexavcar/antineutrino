#ifndef _IO_IN_STREAM
#define _IO_IN_STREAM

namespace neutrino {

class ImageIterator {
public:
  inline ImageIterator(Image &image);
  inline bool has_next();
  inline FObject *next();
  inline void reset();
private:
  word *cursor() { return cursor_; }
  word *limit() { return limit_; }
  Image &image() { return image_; }
  Image &image_;
  word *cursor_;
  word *limit_;
};

} // neutrino

#endif // _IO_IN_STREAM
