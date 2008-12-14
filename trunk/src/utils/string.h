#ifndef _UTILS_STRING
#define _UTILS_STRING

#include <string.h>

#include "utils/globals.h"
#include "utils/list.h"
#include "utils/misc.h"
#include "utils/types.h"

namespace neutrino {

class string {
public:
  inline string(const char chars[])
      : chars_(chars), length_(::strlen(chars)) { }
  inline string()
      : chars_(NULL), length_(0) { }
  inline string(const char *chars, uword length)
      : chars_(chars), length_(length) { }
  inline uword length() const { return length_; }
  inline uword operator[](uword index) const;
  inline string substring(uword start);
  inline string substring(uword start, uword length);

  /**
   * Returns the characters of this string.  The value returned is
   * a pointer to the internal buffer of this string so it will be
   * deleted when the string is.
   */
  inline array<const char> chars();

  bool operator==(string that) const;
  bool operator!=(string that) const { return !(this->operator==(that)); }
  void dispose();
  void println(FILE *out = NULL);
  void print(FILE *out = NULL);
  bool is_empty() { return chars_ == NULL; }
  static string dup(string arg);
  static bool equals(const char* a, const char* b);

private:
  const char *chars_;
  uword length_;
};

/**
 * Formatter element.  Utility class used to wrap an printf argument
 * together with a type tag.
 */
class variant : public nocopy {
public:
  variant() : tag_(eEmpty) { }
  variant(double v) : tag_(eDouble) { value_.u_double = v; }
  variant(int32_t v) : tag_(eInt) { value_.u_int = v; }
  variant(uint32_t v) : tag_(eInt) { value_.u_int = v; }
  variant(int64_t v) : tag_(eInt) { value_.u_int = static_cast<word>(v); }
  variant(uint64_t v) : tag_(eInt) { value_.u_int = static_cast<word>(v); }
  variant(const char *v) : tag_(eCStr) { value_.u_c_str = v; }
  variant(const string &v) : tag_(eString) { value_.u_string = &v; }
  variant(Data *v) : tag_(eObject) { value_.u_object = v; }
  template <typename T>
  variant(const ref<T> &v) : tag_(eRef) { value_.u_ref = reinterpret_cast<const ref<Value>*>(&v); }
  /**
   * Prints this element on the specified buffer.  If the params
   * string is non-null it will be used to configure how this
   * element is printed.
   */
  void print_on(string_buffer &buf, string params) const;
  void print_int_on(string_buffer &buf, string params) const;
private:
  enum Tag { eInt, eCStr, eString, eDouble, eObject, eRef, eEmpty };
  Tag tag_;
  union {
    word u_int;
    const char *u_c_str;
    const string *u_string;
    const ref<Value> *u_ref;
    double u_double;
    Data *u_object;
  } value_;
};


class var_args {
public:
  var_args(uword size) : size_(size) { }
  uword size() const { return size_; }
  virtual const variant &operator[](uword index) const = 0;
private:
  uword size_;
};


template <int n>
class var_args_impl : public var_args {
public:
  var_args_impl(uword size) : var_args(size) { }
  virtual const variant &operator[](uword index) const { return *(elms_[index]); }
  const variant *elms_[n];
};


static inline var_args_impl<1> elms() {
  return var_args_impl<1>(0);
}


static inline var_args_impl<1> elms(const variant &elm) {
  var_args_impl<1> result(1);
  result.elms_[0] = &elm;
  return result;
}


static inline var_args_impl<2> elms(const variant &elm1,
    const variant &elm2) {
  var_args_impl<2> result(2);
  result.elms_[0] = &elm1;
  result.elms_[1] = &elm2;
  return result;
}


static inline var_args_impl<3> elms(const variant &elm1,
    const variant &elm2, const variant &elm3) {
  var_args_impl<3> result(3);
  result.elms_[0] = &elm1;
  result.elms_[1] = &elm2;
  result.elms_[2] = &elm3;
  return result;
}


static inline var_args_impl<4> elms(const variant &elm1,
    const variant &elm2, const variant &elm3, const variant &elm4) {
  var_args_impl<4> result(4);
  result.elms_[0] = &elm1;
  result.elms_[1] = &elm2;
  result.elms_[2] = &elm3;
  result.elms_[3] = &elm4;
  return result;
}


static inline var_args_impl<5> elms(const variant &elm1,
    const variant &elm2, const variant &elm3, const variant &elm4,
    const variant &elm5) {
  var_args_impl<5> result(5);
  result.elms_[0] = &elm1;
  result.elms_[1] = &elm2;
  result.elms_[2] = &elm3;
  result.elms_[3] = &elm4;
  result.elms_[4] = &elm5;
  return result;
}


static inline var_args_impl<6> elms(const variant &elm1,
    const variant &elm2, const variant &elm3, const variant &elm4,
    const variant &elm5, const variant &elm6) {
  var_args_impl<6> result(6);
  result.elms_[0] = &elm1;
  result.elms_[1] = &elm2;
  result.elms_[2] = &elm3;
  result.elms_[3] = &elm4;
  result.elms_[4] = &elm5;
  result.elms_[5] = &elm6;
  return result;
}


static inline var_args_impl<7> elms(const variant &elm1,
    const variant &elm2, const variant &elm3, const variant &elm4,
    const variant &elm5, const variant &elm6, const variant &elm7) {
  var_args_impl<7> result(7);
  result.elms_[0] = &elm1;
  result.elms_[1] = &elm2;
  result.elms_[2] = &elm3;
  result.elms_[3] = &elm4;
  result.elms_[4] = &elm5;
  result.elms_[5] = &elm6;
  result.elms_[6] = &elm7;
  return result;
}


/**
 * An extensible buffer for building up strings.
 */
class string_buffer : public nocopy {
public:
  string_buffer(uword capacity = 16);
  ~string_buffer();
  bool is_empty() { return cursor_ == 0; }
  void append(string str);
  void append(char c);
  void clear();

  /**
   * Creates and return a copy of the contents of this buffer.
   */
  string to_string();

  /**
   * Returns a string backed by the raw internal data of this buffer,
   * which avoids copying.
   */
  string raw_string();

  /**
   * Writes the specified string to this buffer, formatted using the
   * specified elements.
   */
  void printf(string format, const var_args &args);

private:
  void ensure_capacity(int required);
  char *data_;
  uword capacity_;
  uword cursor_;
};

}

#endif // _UTILS_STRING
