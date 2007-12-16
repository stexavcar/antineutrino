#ifndef _UTILS_STRING
#define _UTILS_STRING

#include "utils/globals.h"
#include "utils/types.h"

namespace neutrino {

class string {
public:
  inline string(const char *chars)
      : chars_(chars), length_(length(chars)) { }
  inline string()
      : chars_(NULL), length_(0) { }
  inline string(const char *chars, uint32_t length)
      : chars_(chars), length_(length) { }
  inline uint32_t length() { return length_; }
  inline uint32_t operator[](uint32_t index);
  
  /**
   * Returns the characters of this string.  The value returned is
   * a pointer to the internal buffer of this string so it will be
   * deleted when the string is.
   */
  inline const char *chars() { return chars_; }
  
  bool operator==(string that);
  void dispose();
  void println();
  bool is_empty() { return chars_ == NULL; }
  static uint32_t length(const char* chars);
  static string dup(string arg);
  static bool equals(const char* a, const char* b);
private:
  const char *chars_;
  uint32_t length_;
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
 * An extensible buffer for building up strings.
 */
class string_buffer {
public:
  string_buffer(uint32_t capacity = 16);
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

  class element {
  public:
    element(double value) : tag_(DOUBLE_TAG) { value_.u_double = value; }
    element(int32_t value) : tag_(INT_TAG) { value_.u_int = value; }
    element(uint32_t value) : tag_(INT_TAG) { value_.u_int = value; }
    element(const char *value) : tag_(STRING_TAG) { value_.u_string = value; }
    /**
     * Prints this element on the specified buffer.  If the params
     * string is non-null it will be used to configure how this
     * element is printed.  The string must have two characters
     * available before index 0 for this element to override during
     * printing.
     */
    void print_on(string_buffer &buf, char *params, int offset);
  private:
    enum Tag { INT_TAG, STRING_TAG, DOUBLE_TAG };
    Tag tag_;
    union {
      int32_t u_int;
      const char *u_string;
      double u_double;
    } value_;
  };

  /**
   * Writes the specified string to this buffer, formatted using the
   * specified elements.
   */
  void printf(string format, uint32_t argc, element argv[]);
  
  /**
   * Convenience methods for calling printf.  These allow printf to be
   * called directly with integers, strings, and what have you,
   * because they are implicitly wrapped by the element constructors.
   */
  void printf(string format, element arg1);
  void printf(string format, element arg1, element arg2);
  void printf(string format, element arg1, element arg2,
      element arg3);
  void printf(string format, element arg1, element arg2,
      element arg3, element arg4);

private:
  void ensure_capacity(int required);
  char *data_;
  uint32_t capacity_;
  uint32_t cursor_;
};

}

#endif // _UTILS_STRING
