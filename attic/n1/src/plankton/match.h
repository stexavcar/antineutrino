#ifndef _PLANKTON_MATCH
#define _PLANKTON_MATCH

namespace neutrino {
namespace plankton {

template <word L> class ArrayPattern;

class Pattern {
private:
  enum Type { ptInteger, ptIntegerOut, ptCStr, ptCStrOut, ptArray };
public:
  Pattern(word value) : type_(ptInteger) {
    data_.u_integer = value;
  }
  Pattern(word *value) : type_(ptIntegerOut) {
    data_.u_integer_out = value;
  }
  Pattern(const char *str) : type_(ptCStr) {
    data_.u_c_str = str;
  }
  Pattern(own_vector<char> &str_out) : type_(ptCStrOut) {
    data_.u_c_str_out = &str_out;
  }
  Pattern(word length, const Pattern **elms) : type_(ptArray) {
    data_.u_array.length_ = length;
    data_.u_array.elms_ = elms;
  }
  static inline ArrayPattern<1> of(const Pattern &);
  static inline ArrayPattern<2> of(const Pattern &, const Pattern &);
  static inline ArrayPattern<3> of(const Pattern &, const Pattern &,
      const Pattern &);
  static inline ArrayPattern<4> of(const Pattern &, const Pattern &,
      const Pattern &, const Pattern &);
  static inline ArrayPattern<5> of(const Pattern &, const Pattern &,
      const Pattern &, const Pattern &, const Pattern &);
  static inline ArrayPattern<6> of(const Pattern &, const Pattern &,
      const Pattern &, const Pattern &, const Pattern &,
      const Pattern &);
  bool match(Value value) const;
private:
  Type type_;
  union {
    word u_integer;
    word *u_integer_out;
    const char *u_c_str;
    own_vector<char> *u_c_str_out;
    struct {
      word length_;
      const Pattern **elms_;
    } u_array;
  } data_;
};

template <word L>
class ArrayPattern : public Pattern {
public:
  ArrayPattern() : Pattern(L, elms_) { }
private:
  friend class Pattern;
  const Pattern *elms_[L];
};

ArrayPattern<1> Pattern::of(const Pattern &e0) {
  ArrayPattern<1> result;
  result.elms_[0] = &e0;
  return result;
}

ArrayPattern<2> Pattern::of(const Pattern &e0, const Pattern &e1) {
  ArrayPattern<2> result;
  result.elms_[0] = &e0;
  result.elms_[1] = &e1;
  return result;
}

ArrayPattern<3> Pattern::of(const Pattern &e0, const Pattern &e1,
    const Pattern &e2) {
  ArrayPattern<3> result;
  result.elms_[0] = &e0;
  result.elms_[1] = &e1;
  result.elms_[2] = &e2;
  return result;
}

ArrayPattern<4> Pattern::of(const Pattern &e0, const Pattern &e1,
    const Pattern &e2, const Pattern &e3) {
  ArrayPattern<4> result;
  result.elms_[0] = &e0;
  result.elms_[1] = &e1;
  result.elms_[2] = &e2;
  result.elms_[3] = &e3;
  return result;
}

ArrayPattern<5> Pattern::of(const Pattern &e0, const Pattern &e1,
    const Pattern &e2, const Pattern &e3, const Pattern &e4) {
  ArrayPattern<5> result;
  result.elms_[0] = &e0;
  result.elms_[1] = &e1;
  result.elms_[2] = &e2;
  result.elms_[3] = &e3;
  result.elms_[4] = &e4;
  return result;
}

ArrayPattern<6> Pattern::of(const Pattern &e0, const Pattern &e1,
    const Pattern &e2, const Pattern &e3, const Pattern &e4,
    const Pattern &e5) {
  ArrayPattern<6> result;
  result.elms_[0] = &e0;
  result.elms_[1] = &e1;
  result.elms_[2] = &e2;
  result.elms_[3] = &e3;
  result.elms_[4] = &e4;
  result.elms_[5] = &e5;
  return result;
}

} // plankton
} // neutrino

#endif // _PLANKTON_MATCH
