#include "plankton/plankton-inl.h"
#include "plankton/match.h"
#include "utils/string-inl.h"
#include "utils/hash-map-inl.h"

namespace neutrino {
namespace plankton {

bool Pattern::match(Value value) const {
  switch (type_) {
  case ptInteger:
    if (!is<Integer>(value)) return false;
    return cast<Integer>(value).value() == data_.u_integer;
  case ptIntegerOut:
    if (!is<Integer>(value)) return false;
    *data_.u_integer_out = cast<Integer>(value).value();
    return true;
  case ptCStr:
    if (!is<String>(value)) return false;
    return cast<String>(value) == data_.u_c_str;
  case ptCStrOut: {
    if (!is<String>(value)) return false;
    String str = cast<String>(value);
    word length = str.length();
    own_vector<char> &result = *data_.u_c_str_out;
    result.set(vector<char>::allocate(length + 1));
    for (word i = 0; i < length; i++)
      result[i] = str[i];
    result[length] = '\0';
    return true;
  }
  case ptArray: {
    if (!is<Array>(value)) return false;
    Array arr = cast<Array>(value);
    word length = arr.length();
    if (length != data_.u_array.length_)
      return false;
    for (word i = 0; i < length; i++) {
      if (!data_.u_array.elms_[i]->match(arr[i]))
        return false;
    }
    return true;
  }
  default:
    assert false;
    return false;
  }
}

} // namespace plankton
} // namespace neutrino
