#ifndef _PLANKTON_PLANKTON
#define _PLANKTON_PLANKTON

#include "utils/global.h"
#include "utils/string.h"

namespace positron {

class p_value {
public:
  enum Type { vtInteger, vtString, vtNull, vtArray };

  struct DTable {
    Type (*value_type)(const p_value *that);
    bool (*value_eq)(const p_value *that, p_value other);
    int32_t (*integer_value)(const p_integer *that);
    word (*string_length)(const p_string *that);
    uint32_t (*string_get)(const p_string *that, word offset);
    word (*array_length)(const p_array *that);
    p_value (*array_get)(const p_array *that, word index);
    bool (*array_set)(p_array *that, word index, p_value value);
    word (*string_compare)(const p_string *that, const string &other);
  };

  inline Type type() const { return dtable().value_type(this); }
  inline bool operator==(p_value that) const { return dtable().value_eq(this, that); }
  uint32_t data() const { return data_; }
  DTable &dtable() const { return dtable_; }

  inline p_value(uint32_t data, DTable &dtable) : data_(data), dtable_(dtable) { }

private:
  uint32_t data_;
  DTable &dtable_;
};


template <> struct coerce<p_value::Type> { typedef word type; };


class p_integer : public p_value {
public:
  inline int32_t value() const { return dtable().integer_value(this); }
  inline p_integer(uint32_t data, DTable &dtable) : p_value(data, dtable) { }
  static const p_value::Type kTypeTag = p_value::vtInteger;
};


class p_null : public p_value {
public:
  inline p_null(uint32_t data, DTable &dtable) : p_value(data, dtable) { }
  static const p_value::Type kTypeTag = p_value::vtNull;
};


class p_string : public p_value {
public:
  inline word length() const { return dtable().string_length(this); }
  inline uint32_t operator[](word index) const { return dtable().string_get(this, index); }
  inline bool operator==(const string &other) const { return dtable().string_compare(this, other) == 0; }
  inline bool operator!=(const string &other) const { return dtable().string_compare(this, other) != 0; }
  inline bool operator<(const string &other) const { return dtable().string_compare(this, other) < 0; }
  inline bool operator<=(const string &other) const { return dtable().string_compare(this, other) <= 0; }
  inline bool operator>(const string &other) const { return dtable().string_compare(this, other) > 0; }
  inline bool operator>=(const string &other) const { return dtable().string_compare(this, other) >= 0; }
  inline p_string(uint32_t data, DTable &dtable) : p_value(data, dtable) { }
  static const p_value::Type kTypeTag = p_value::vtString;
};


class p_array : public p_value {
public:
  inline word length() const { return dtable().array_length(this); }
  inline p_value operator[](word index) const { return dtable().array_get(this, index); }
  inline bool set(word index, p_value value) { return dtable().array_set(this, index, value); }
  inline p_array(uint32_t data, DTable &dtable) : p_value(data, dtable) { }
  static const p_value::Type kTypeTag = p_value::vtArray;
};


template <class T>
static inline bool is(p_value obj);


template <class T>
static inline T cast(p_value obj);


} // namespace positron

#endif // _PLANKTON_PLANKTON
