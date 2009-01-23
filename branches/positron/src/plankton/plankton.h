#ifndef _PLANKTON_PLANKTON
#define _PLANKTON_PLANKTON

#include "utils/global.h"
#include "utils/string.h"

namespace neutrino {

class p_value {
public:
  enum Type { vtInteger, vtString, vtNull, vtVoid, vtArray, vtObject };

  struct DTable {
    struct ValueDTable {
      Type (*type)(const p_value *that);
      bool (*eq)(const p_value *that, p_value other);
      void *(*impl_id)(const p_value *that);
    };
    struct IntegerDTable {
      int32_t (*value)(const p_integer *that);
    };
    struct StringDTable {
      word (*length)(const p_string *that);
      uint32_t (*get)(const p_string *that, word offset);
      word (*compare)(const p_string *that, const string &other);
    };
    struct ArrayDTable {
      word (*length)(const p_array *that);
      p_value (*get)(const p_array *that, word index);
      bool (*set)(p_array *that, word index, p_value value);
    };
    struct ObjectDTable {
      p_value (*send)(const p_object *that, p_string name, p_array args,
          bool is_synchronous);
    };
    ValueDTable value;
    IntegerDTable integer;
    StringDTable string;
    ArrayDTable array;
    ObjectDTable object;
  };

  inline Type type() const { return dtable()->value.type(this); }
  inline void *impl_id() const { return dtable()->value.impl_id(this); }
  inline bool operator==(p_value that) const { return dtable()->value.eq(this, that); }
  word data() const { return data_; }
  DTable *dtable() const { return dtable_; }
  bool is_empty() { return dtable_ == NULL; }

  inline p_value(word data, DTable *dtable) : data_(data), dtable_(dtable) { }
  inline p_value() : data_(0), dtable_(NULL) { }

private:
  word data_;
  DTable *dtable_;
};


template <> struct coerce<p_value::Type> { typedef word type; };


class p_integer : public p_value {
public:
  inline int32_t value() const { return dtable()->integer.value(this); }
  inline p_integer(word data, DTable *dtable) : p_value(data, dtable) { }
  static const p_value::Type kTypeTag = p_value::vtInteger;
};


class p_null : public p_value {
public:
  inline p_null(word data, DTable *dtable) : p_value(data, dtable) { }
  static const p_value::Type kTypeTag = p_value::vtNull;
};


class p_void : public p_value {
public:
  inline p_void(word data, DTable *dtable) : p_value(data, dtable) { }
  static const p_value::Type kTypeTag = p_value::vtVoid;
};


class p_string : public p_value {
public:
  inline word length() const { return dtable()->string.length(this); }
  inline uint32_t operator[](word index) const { return dtable()->string.get(this, index); }
  inline bool operator==(const string &other) const { return dtable()->string.compare(this, other) == 0; }
  inline bool operator!=(const string &other) const { return dtable()->string.compare(this, other) != 0; }
  inline bool operator<(const string &other) const { return dtable()->string.compare(this, other) < 0; }
  inline bool operator<=(const string &other) const { return dtable()->string.compare(this, other) <= 0; }
  inline bool operator>(const string &other) const { return dtable()->string.compare(this, other) > 0; }
  inline bool operator>=(const string &other) const { return dtable()->string.compare(this, other) >= 0; }
  inline p_string(word data, DTable *dtable) : p_value(data, dtable) { }
  inline p_string() : p_value() { }
  static const p_value::Type kTypeTag = p_value::vtString;
};


class p_array : public p_value {
public:
  inline word length() const { return dtable()->array.length(this); }
  inline p_value operator[](word index) const { return dtable()->array.get(this, index); }
  inline bool set(word index, p_value value) { return dtable()->array.set(this, index, value); }
  inline p_array(word data, DTable *dtable) : p_value(data, dtable) { }
  inline p_array() : p_value() { }
  static const p_value::Type kTypeTag = p_value::vtArray;
};


class p_object : public p_value {
public:
  inline p_value send_sync(p_string name, p_array args = p_array()) { return dtable()->object.send(this, name, args, true); }
  inline void send_async(p_string name, p_array args = p_array()) { dtable()->object.send(this, name, args, false); }
  inline p_object(word data, DTable *table) : p_value(data, table) { }
  static const p_value::Type kTypeTag = p_value::vtObject;
};


template <class T>
static inline bool is(p_value obj);


template <class T>
static inline T cast(p_value obj);


} // namespace neutrino

#endif // _PLANKTON_PLANKTON
