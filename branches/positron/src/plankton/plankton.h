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
    size_t (*string_length)(String *that);
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
  inline size_t length() { return dtable().string_length(this); }
  inline String(void *data, DTable &dtable) : Value(data, dtable) { }
};


} // namespace plankton
} // namespace positron

#endif // _PLANKTON_PLANKTON
