#ifndef _PLANKTON_BUILDER
#define _PLANKTON_BUILDER

#include "plankton/plankton.h"
#include "utils/array.h"
#include "utils/buffer.h"
#include "utils/string.h"

namespace positron {

class DTableImpl : public p_value::DTable {
public:
  DTableImpl(MessageBuffer *builder);
  static DTableImpl &static_instance() { return kStaticInstance; }
  MessageBuffer &builder() { return *builder_; }
private:
  MessageBuffer *builder_;
  static DTableImpl kStaticInstance;
};

class object {
public:
  object(array<uint8_t> data) : data_(data) { }
  uint8_t *start() { return data().start(); }
  template <typename T>
  inline T &at(word offset);
  array<uint8_t> data() { return data_; }
private:
  array<uint8_t> data_;
};

class ISocket {
public:
  virtual word write(const vector<uint8_t> &data) = 0;
  virtual word read(vector<uint8_t> &data) = 0;
};

class MessageBuffer {
public:
  MessageBuffer();
  static p_integer new_integer(int32_t value);
  static p_null get_null();
  p_string new_string(string value);
  p_array new_array(word length);

  object resolve(word offset);
  buffer<uint8_t> &data() { return data_; }

  // Sends the specified value on the specified socket.  Note that all
  // objects created with this builder will be sent, potentially
  // including objects that are not part of the specified value.
  bool send(p_value value, ISocket &socket);
  p_value receive(ISocket &socket);

private:
  template <typename T> T to_plankton(object &obj);
  object allocate(p_value::Type type, word size);
  DTableImpl &dtable() { return dtable_; }
  DTableImpl dtable_;
  buffer<uint8_t> data_;
};

class Raw {
public:
  static inline uint32_t tag_object(word offset);
  static inline word untag_object(uint32_t data);
  static inline bool has_object_tag(uint32_t data);
  static inline uint32_t tag_integer(word value);
  static inline word untag_integer(uint32_t data);
  static inline bool has_integer_tag(uint32_t data);
  static inline uint32_t tag_singleton(p_value::Type type);
  static inline p_value::Type untag_singleton(uint32_t data);
  static inline bool has_singleton_tag(uint32_t data);
  static const word kTagSize = 2;
  static const word kTagMask = (1 << kTagSize) - 1;
  static const uword kIntegerTag = 0;
  static const uword kObjectTag = 1;
  static const uword kSingletonTag = 2;
};

} // namespace positron

#endif // _PLANKTON_BUILDER
