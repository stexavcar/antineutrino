#include "plankton/builder.h"

namespace positron {

class ValueImpl : public plankton::Value {
public:
  Type type();
};

class IntegerImpl : public plankton::Integer {
public:
  int32_t value();
};

plankton::Value::Type ValueImpl::type() {
  word value = reinterpret_cast<word>(data());
  if ((value & 1) == 0) {
    return vtInteger;
  } else {
    return vtInteger;
  }
}

int32_t IntegerImpl::value() {
  return reinterpret_cast<word>(data()) >> 1;
}

class StaticDTable : public plankton::Value::DTable {
public:
  StaticDTable();
  static StaticDTable &instance() { return kInstance; }
private:
  static StaticDTable kInstance;
};

StaticDTable StaticDTable::kInstance;

StaticDTable::StaticDTable() {
  value_type = static_cast<plankton::Value::Type (plankton::Value::*)()>(&ValueImpl::type);
  integer_value = static_cast<int32_t (plankton::Integer::*)()>(&IntegerImpl::value);
}

plankton::Integer Builder::new_integer(int32_t value) {
  return plankton::Integer(reinterpret_cast<void*>(value << 1),
      StaticDTable::instance());
}

} // namespace positron
