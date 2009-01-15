#ifndef _PLANKTON_PLANKTON
#define _PLANKTON_PLANKTON

#include "utils/global.h"

namespace positron {
namespace plankton {

class Integer;

class Value {
public:
  enum Type { vtInteger };

  struct DTable {
    Type (Value::*value_type)();
    int32_t (Integer::*integer_value)();
  };

  inline Type type() { return (this->*(dtable().value_type))(); }

  inline Value(void *data, DTable &dtable) : data_(data), dtable_(dtable) { }

protected:
  void *data() { return data_; }
  DTable &dtable() { return dtable_; }

private:
  void *data_;
  DTable &dtable_;
};


class Integer : public Value {
public:
  inline int32_t value() { return (this->*(dtable().integer_value))(); }
  inline Integer(void *data, DTable &dtable) : Value(data, dtable) { }
};


} // namespace plankton
} // namespace positron

#endif // _PLANKTON_PLANKTON
