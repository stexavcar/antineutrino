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
      : chars_(chars), length_(IF_ELSE_DEBUG(kNoLength, ::strlen(chars))) { }
  inline string()
      : chars_(NULL), length_(0) { }
  inline string(const char *chars, uword length)
      : chars_(chars), length_(length) { }
  inline uword length() {
    IF_DEBUG(if (length_ == kNoLength) length_ = ::strlen(chars_));
    return length_;
  }
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
  bool is_empty() { return chars_ == NULL; }
  static string dup(string arg);
  static bool equals(const char* a, const char* b);
  
  static const uword kNoLength = ~0;
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
  fmt_elm(double value) : tag_(eDouble) { value_.u_double = value; }
  fmt_elm(uint16_t value) : tag_(eInt) { value_.u_int = value; }
  fmt_elm(int16_t value) : tag_(eInt) { value_.u_int = value; }
  fmt_elm(uint32_t value) : tag_(eInt) { value_.u_int = value; }
  fmt_elm(int32_t value) : tag_(eInt) { value_.u_int = value; }
  fmt_elm(uint64_t value) : tag_(eInt) { value_.u_int = static_cast<word>(value); }
  fmt_elm(int64_t value) : tag_(eInt) { value_.u_int = static_cast<word>(value); }
  fmt_elm(const char *value) : tag_(eString) { value_.u_string = value; }
  fmt_elm(string value) : tag_(eString) { value_.u_string = value.chars(); }
  fmt_elm(Data *value) : tag_(eObject) { value_.u_object = value; }
  /**
   * Prints this element on the specified buffer.  If the params
   * string is non-null it will be used to configure how this
   * element is printed.
   */
  void print_on(string_buffer &buf, string params);
  void print_int_on(string_buffer &buf, string params);
private:
  enum Tag { eInt, eString, eDouble, eObject };
  Tag tag_;
  union {
    word u_int;
    const char *u_string;
    double u_double;
    Data *u_object;
  } value_;
};

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
  void printf(string format, list<fmt_elm> args);
  
  /**
   * Convenience methods for calling printf.  These allow printf to be
   * called directly with integers, strings, and what have you,
   * because they are implicitly wrapped by the element constructors.
   */
  void printf(string format, fmt_elm arg1);
  void printf(string format, fmt_elm arg1, fmt_elm arg2);
  void printf(string format, fmt_elm arg1, fmt_elm arg2, fmt_elm arg3);
  void printf(string format, fmt_elm arg1, fmt_elm arg2, fmt_elm arg3,
      fmt_elm arg4);
  void printf(string format, fmt_elm arg1, fmt_elm arg2, fmt_elm arg3,
      fmt_elm arg4, fmt_elm arg5);
  void printf(string format, fmt_elm arg1, fmt_elm arg2, fmt_elm arg3,
      fmt_elm arg4, fmt_elm arg5, fmt_elm arg6);
  void printf(string format, fmt_elm arg1, fmt_elm arg2, fmt_elm arg3,
      fmt_elm arg4, fmt_elm arg5, fmt_elm arg6, fmt_elm arg7);

private:
  void ensure_capacity(int required);
  char *data_;
  uword capacity_;
  uword cursor_;
};

}

#endif // _UTILS_STRING
