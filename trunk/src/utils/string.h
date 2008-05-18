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
  inline uword length() { return length_; }
  inline uword operator[](uword index);
  inline string substring(uword start);
  inline string substring(uword start, uword length);
  
  /**
   * Returns the characters of this string.  The value returned is
   * a pointer to the internal buffer of this string so it will be
   * deleted when the string is.
   */
  inline const char *chars() { return chars_; }
  
  bool operator==(string that);
  bool operator!=(string that) { return !(this->operator==(that)); }
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

class scoped_string {
public:
  scoped_string(string str) : value_(str) { }
  ~scoped_string() { value_.dispose(); }
  string &operator*() { return value_; }
  string *operator->() { return &value_; }
  const char *chars() { return value_.chars(); }
private:
  string value_;
};


/**
 * Formatter element.  Utility class used to wrap an printf argument
 * together with a type tag.
 */
class fmt_elm {
public:
  fmt_elm() : tag_(eEmpty) { }
  fmt_elm(double value) : tag_(eDouble) { value_.u_double = value; }
  fmt_elm(uint16_t value) : tag_(eInt) { value_.u_int = value; }
  fmt_elm(int16_t value) : tag_(eInt) { value_.u_int = value; }
  fmt_elm(uint32_t value) : tag_(eInt) { value_.u_int = value; }
  fmt_elm(int32_t value) : tag_(eInt) { value_.u_int = value; }
  fmt_elm(uint64_t value) : tag_(eInt) { value_.u_int = static_cast<word>(value); }
  fmt_elm(int64_t value) : tag_(eInt) { value_.u_int = static_cast<word>(value); }
  fmt_elm(const char *value) : tag_(eCStr) { value_.u_c_str = value; }
  fmt_elm(const string &value) : tag_(eString) { value_.u_string = &value; }
  fmt_elm(Data *value) : tag_(eObject) { value_.u_object = value; }
  /**
   * Prints this element on the specified buffer.  If the params
   * string is non-null it will be used to configure how this
   * element is printed.
   */
  void print_on(string_buffer &buf, string params) const;
  void print_int_on(string_buffer &buf, string params) const;
private:
  enum Tag { eInt, eCStr, eString, eDouble, eObject, eEmpty };
  Tag tag_;
  union {
    word u_int;
    const char *u_c_str;
    const string *u_string;
    double u_double;
    Data *u_object;
  } value_;
};


class fmt_elms {
public:
  fmt_elms(uword size) : size_(size) { }
  uword size() const { return size_; }
  virtual const fmt_elm &operator[](uword index) const = 0;
private:
  uword size_;  
};


template <int n>
class fmt_elms_impl : public fmt_elms {
public:
  fmt_elms_impl(uword size) : fmt_elms(size) { }
  virtual const fmt_elm &operator[](uword index) const { return *(elms_[index]); }
  const fmt_elm *elms_[n];
};


static inline fmt_elms_impl<1> elms() {
  return fmt_elms_impl<1>(0);
}


static inline fmt_elms_impl<1> elms(const fmt_elm &elm) {
  fmt_elms_impl<1> result(1);
  result.elms_[0] = &elm;
  return result;
}


static inline fmt_elms_impl<2> elms(const fmt_elm &elm1,
    const fmt_elm &elm2) {
  fmt_elms_impl<2> result(2);
  result.elms_[0] = &elm1;
  result.elms_[1] = &elm2;
  return result;
}


static inline fmt_elms_impl<3> elms(const fmt_elm &elm1,
    const fmt_elm &elm2, const fmt_elm &elm3) {
  fmt_elms_impl<3> result(3);
  result.elms_[0] = &elm1;
  result.elms_[1] = &elm2;
  result.elms_[2] = &elm3;
  return result;
}


static inline fmt_elms_impl<4> elms(const fmt_elm &elm1,
    const fmt_elm &elm2, const fmt_elm &elm3, const fmt_elm &elm4) {
  fmt_elms_impl<4> result(4);
  result.elms_[0] = &elm1;
  result.elms_[1] = &elm2;
  result.elms_[2] = &elm3;
  result.elms_[3] = &elm4;
  return result;
}


static inline fmt_elms_impl<5> elms(const fmt_elm &elm1,
    const fmt_elm &elm2, const fmt_elm &elm3, const fmt_elm &elm4,
    const fmt_elm &elm5) {
  fmt_elms_impl<5> result(5);
  result.elms_[0] = &elm1;
  result.elms_[1] = &elm2;
  result.elms_[2] = &elm3;
  result.elms_[3] = &elm4;
  result.elms_[4] = &elm5;
  return result;
}


static inline fmt_elms_impl<6> elms(const fmt_elm &elm1,
    const fmt_elm &elm2, const fmt_elm &elm3, const fmt_elm &elm4,
    const fmt_elm &elm5, const fmt_elm &elm6) {
  fmt_elms_impl<6> result(6);
  result.elms_[0] = &elm1;
  result.elms_[1] = &elm2;
  result.elms_[2] = &elm3;
  result.elms_[3] = &elm4;
  result.elms_[4] = &elm5;
  result.elms_[5] = &elm6;
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
  void printf(string format, const fmt_elms &args);
  
private:
  void ensure_capacity(int required);
  char *data_;
  uword capacity_;
  uword cursor_;
};

}

#endif // _UTILS_STRING
