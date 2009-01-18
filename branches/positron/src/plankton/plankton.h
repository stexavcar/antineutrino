#ifndef _PLANKTON_PLANKTON
#define _PLANKTON_PLANKTON

#include "utils/global.h"

namespace positron {
namespace plankton {

class Integer;
class String;

class Value {
public:
  enum Type { vtInteger, vtString };

  struct DTable {
    Type (*value_type)(Value *that);
    int32_t (*integer_value)(Integer *that);
    word (*string_length)(String *that);
    uint32_t (*string_get)(String *that, word offset);
  };

  inline Type type() { return dtable().value_type(this); }

  inline Value(void *data, DTable &dtable) : data_(data), dtable_(dtable) { }

  void *data() { return data_; }
  DTable &dtable() { return dtable_; }

private:
  void *data_;
  DTable &dtable_;
};


class Integer : public Value {
public:
  inline int32_t value() { return dtable().integer_value(this); }
  inline Integer(void *data, DTable &dtable) : Value(data, dtable) { }
};


class String : public Value {
public:
  inline word length() { return dtable().string_length(this); }
  inline uint32_t operator[](word index) { return dtable().string_get(this, index); }
  inline String(void *data, DTable &dtable) : Value(data, dtable) { }
};


} // namespace plankton
} // namespace positron

#endif // _PLANKTON_PLANKTON
