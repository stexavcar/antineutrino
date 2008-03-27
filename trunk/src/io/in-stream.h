#ifndef _IO_IN_STREAM
#define _IO_IN_STREAM

namespace neutrino {

template <class Provider, class Data>
class ImageIterator {
public:
  inline ImageIterator(Data &heap);
  inline bool has_next();
  inline FObject *next();
  inline void reset();
private:
  uword cursor() { return cursor_; }
  Data &heap_;
  uword cursor_;
};

class FixedHeap {
public:
  typedef list<word> Data;
  static inline word *address(Data &data, uword cursor);
  static inline bool has_more(Data &data, uword cursor);
};

class ExtensibleHeap {
public:
  typedef list_buffer<word> Data;
  static inline word *address(Data &data, uword cursor);
  static inline bool has_more(Data &data, uword cursor);
};

} // neutrino

#endif // _IO_IN_STREAM
