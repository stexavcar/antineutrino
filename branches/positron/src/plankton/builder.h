#ifndef _PLANKTON_BUILDER
#define _PLANKTON_BUILDER

#include "plankton/plankton.h"
#include "utils/array.h"
#include "utils/buffer.h"
#include "utils/string.h"
#include "utils/smart-ptrs.h"
#include "value/condition.h"

namespace neutrino {

class DTableImpl : public p_value::DTable {
public:
  DTableImpl(MessageHeap *heap);
  static DTableImpl &static_instance() { return kStaticInstance; }
  MessageHeap &heap() { return *heap_; }
private:
  MessageHeap *heap_;
  static DTableImpl kStaticInstance;
};

class FrozenObject {
public:
  FrozenObject(array<uint8_t> data) : data_(data) { }
  uint8_t *start() { return data().start(); }
  template <typename T>
  inline T &at(word offset);
  array<uint8_t> data() { return data_; }
private:
  array<uint8_t> data_;
};

class MessageHeap {
public:
  MessageHeap();
  virtual ~MessageHeap() { }
  virtual vector<uint8_t> memory() = 0;
  FrozenObject resolve(word offset);
  void take_ownership(MessageHeap *that);
  template <typename T> T to_plankton(FrozenObject &obj);

  static void *id() { return &DTableImpl::static_instance(); }
  static MessageHeap *get(p_value::DTable *dtable);
  DTableImpl &dtable() { return dtable_; }
private:
  own_ptr<MessageHeap> owned_;
  DTableImpl dtable_;
};

class FrozenHeap : public MessageHeap {
public:
  FrozenHeap(vector<uint8_t> data) : data_(data) { }
  virtual vector<uint8_t> memory() { return data(); }
private:
  vector<uint8_t> data() { return data_.as_vector(); }
  own_vector<uint8_t> data_;
};

class MessageOut : public MessageHeap {
public:
  static p_integer new_integer(int32_t value);
  static p_null get_null();
  static p_void get_void();
  p_string new_string(string value);
  p_array new_array(word length);

  buffer<uint8_t> &data() { return data_; }
  virtual vector<uint8_t> memory();

private:
  FrozenObject allocate(p_value::Type type, word size);
  buffer<uint8_t> data_;
};

class IStream {
public:
  virtual boole send_reply(MessageIn &message, p_value value) = 0;
};

class MessageIn {
public:
  MessageIn() : stream_(NULL), is_synchronous_(false), has_replied_(false) { }
  inline ~MessageIn();
  boole reply(p_value value);
  void set_selector(p_string v) { selector_ = v; }
  p_string selector() { return selector_; }
  void set_args(p_array v) { args_ = v; }
  p_array args() { return args_; }
  void set_stream(IStream *v) { stream_ = v; }
  void set_is_synchronous(bool v) { is_synchronous_ = v; }
  bool is_synchronous() { return is_synchronous_; }
  void take_ownership(MessageHeap *that);
private:
  IStream *stream_;
  p_string selector_;
  p_array args_;
  bool is_synchronous_;
  bool has_replied_;
  own_ptr<MessageHeap> owned_;
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

} // namespace neutrino

#endif // _PLANKTON_BUILDER
