#ifndef _PLANKTON_BUILDER
#define _PLANKTON_BUILDER

#include "plankton/plankton.h"
#include "utils/array.h"
#include "utils/buffer.h"
#include "utils/string.h"
#include "utils/smart-ptrs.h"
#include "value/condition.h"

namespace neutrino {

class MiniHeapDTable : public p::Value::DTable {
public:
  MiniHeapDTable(MiniHeap *heap);
  static MiniHeapDTable &static_instance() { return kStaticInstance; }
  MiniHeap &heap() { return *heap_; }
private:
  MiniHeap *heap_;
  static MiniHeapDTable kStaticInstance;
};

class MiniHeapObject {
public:
  MiniHeapObject(array<uint8_t> data) : data_(data) { }
  uint8_t *start() { return data().start(); }
  template <typename T>
  inline T &at(word offset);
  array<uint8_t> data() { return data_; }
private:
  array<uint8_t> data_;
};

class MiniHeap {
public:
  MiniHeap();
  virtual ~MiniHeap() { }
  virtual vector<uint8_t> memory() = 0;
  MiniHeapObject resolve(word offset);
  void take_ownership(MiniHeap *that);
  template <typename T> T to_plankton(MiniHeapObject &obj);

  static void *id() { return &MiniHeapDTable::static_instance(); }
  static MiniHeap *get(p::Value::DTable *dtable);
  MiniHeapDTable &dtable() { return dtable_; }
private:
  own_ptr<MiniHeap> owned_;
  MiniHeapDTable dtable_;
};

class FrozenMiniHeap : public MiniHeap {
public:
  FrozenMiniHeap(vector<uint8_t> data) : data_(data) { }
  virtual vector<uint8_t> memory() { return data(); }
private:
  vector<uint8_t> data() { return data_.as_vector(); }
  own_vector<uint8_t> data_;
};

class Factory : public MiniHeap, public p::IMessageResource {
public:
  p::String new_string(string value);
  p::Array new_array(word length);

  static p::Integer new_integer(int32_t value);
  static p::Null get_null();
  static p::Void get_void();

  buffer<uint8_t> &data() { return data_; }
  virtual vector<uint8_t> memory();

private:
  MiniHeapObject allocate(p::Value::Type type, word size);
  buffer<uint8_t> data_;
};

class MessageOut : public Factory {
public:
private:
};

class IStream {
public:
  virtual boole send_reply(MessageIn &message, p::Value value) = 0;
};

class MessageIn {
public:
  MessageIn() : stream_(NULL), is_synchronous_(false), has_replied_(false) { }
  inline ~MessageIn();
  boole reply(p::Value value);
  void set_selector(p::String v) { selector_ = v; }
  p::String selector() { return selector_; }
  void set_args(p::Array v) { args_ = v; }
  p::Array args() { return args_; }
  void set_stream(IStream *v) { stream_ = v; }
  void set_is_synchronous(bool v) { is_synchronous_ = v; }
  bool is_synchronous() { return is_synchronous_; }
  void take_ownership(MiniHeap *that);
private:
  IStream *stream_;
  p::String selector_;
  p::Array args_;
  bool is_synchronous_;
  bool has_replied_;
  own_ptr<MiniHeap> owned_;
};

template <> struct coerce<p::Value::Type> { typedef word type; };

template <class T>
static inline bool is(p::Value obj);

template <class T>
static inline T cast(p::Value obj);

class MiniHeapPointer {
public:
  static inline uint32_t tag_object(word offset);
  static inline word untag_object(uint32_t data);
  static inline bool has_object_tag(uint32_t data);
  static inline uint32_t tag_integer(word value);
  static inline word untag_integer(uint32_t data);
  static inline bool has_integer_tag(uint32_t data);
  static inline uint32_t tag_singleton(p::Value::Type type);
  static inline p::Value::Type untag_singleton(uint32_t data);
  static inline bool has_singleton_tag(uint32_t data);
  static const word kTagSize = 2;
  static const word kTagMask = (1 << kTagSize) - 1;
  static const uword kIntegerTag = 0;
  static const uword kObjectTag = 1;
  static const uword kSingletonTag = 2;
};

// --- O b j e c t s ---

class Message {
public:
  Message(p::Object self, p::String name, p::Array args, p::MessageData *data,
      bool is_synchronous)
    : self_(self)
    , name_(name)
    , args_(args)
    , data_(data)
    , is_synchronous_(is_synchronous) { }
  p::Object self() { return self_; }
  p::String name() { return name_; }
  p::Array args() { return args_; }
  p::MessageData *data() { return data_; }
  bool is_synchronous() { return is_synchronous_; }
private:
  p::Object self_;
  p::String name_;
  p::Array args_;
  p::MessageData *data_;
  bool is_synchronous_;
};

template <typename T>
class ObjectProxyDTable : public p::Value::DTable {
public:
  typedef p::Value (T::*method_t)(Message &message);
  ObjectProxyDTable();
  void add_method(p::String name, method_t method);
  p::Object proxy_for(T &object);
private:
  static p::Value object_send(p::Object self, p::String name, p::Array args,
      p::MessageData *data, bool is_synchronous);
  hash_map<p::String, method_t> &methods() { return methods_; }
  hash_map<p::String, method_t> methods_;
};

} // namespace neutrino

#endif // _PLANKTON_BUILDER
