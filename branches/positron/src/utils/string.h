#ifndef _UTILS_STRING
#define _UTILS_STRING


#include <cstring>
#include <cstdio>


#include "utils/vector.h"
#include "utils/buffer.h"


namespace positron {


class string {
public:
  inline string()
    : chars_(NULL), length_(0) { }
  inline string(const char *chars)
    : chars_(chars)
    , length_(strlen(chars)) { }
  inline string(const char *chars, word length)
    : chars_(chars)
    , length_(length) { }
  bool operator==(const string &that) const;
  bool contains(char c);
  word length() const { return length_; }
  const char *start() { return chars_; }
  const char &operator[](word i) const { return chars_[i]; }
  static string dup(string arg);
  vector<const char> chars() { return vector<const char>(chars_, length_); }
  bool is_empty() { return chars_ == NULL; }
private:
  const char *chars_;
  word length_;
};


class string_stream;


class variant_type {
public:
  virtual void print_on(const void *data, string modifiers,
      string_stream &stream) = 0;
};


template <typename T>
class variant_type_impl : public variant_type {
public:
  static inline const void *encode(const T &t);
};


// This little trick defines some implicit conversions that must be
// performed before wrapping a value in a variant.  The default "type
// conversion" is to leave the type as it is, but additional conversions
// can be added by specializing the Coerce struct.
template <typename T> struct coerce { typedef T type; };
template <> struct coerce<int> { typedef word type; };
template <> struct coerce<unsigned> { typedef word type; };
template <> struct coerce<size_t> { typedef word type; };

class variant {
public:
  template <typename T>
  inline variant(const T &t)
    : type_(variant_type_impl<typename coerce<T>::type>::kInstance)
    , data_(variant_type_impl<typename coerce<T>::type>::encode(t)) { }
  void print_on(string_stream &stream, string modifiers) const;
private:
  variant_type &type() const { return type_; }
  const void *data() const { return data_; }
  variant_type &type_;
  const void *data_;
  int index_;
};


class var_args {
public:
  virtual word length() const = 0;
  virtual const variant &operator[](word i) const = 0;
};


template <word L>
class var_args_impl : public var_args {
public:
  virtual word length() const { return L; }
  virtual const variant &operator[](word i) const;
  embed_vector<const variant*, L> elms_;
};


class string_stream {
public:
  void add(char c);
  void add(string format, const var_args &args);
  void add(string str);

  // Returns a null-terminated string that is backed directly by this
  // string stream's internal character buffer.  The returned string
  // is only valid as long as this string stream exists and until the
  // next call to 'add'.
  string raw_c_str();

  // Sets a string that will be inserted after every newline.
  void set_indent(string str) { indent_ = str; }

private:
  string indent() { return indent_; }
  string indent_;
  buffer<char> &buf() { return buf_; }
  buffer<char> buf_;
};


class format_bundle {
public:
  format_bundle(string format, const var_args &args)
    : format_(format)
    , args_(args) { }
  string format() const { return format_; }
  const var_args &args() const { return args_; }
private:
  string format_;
  const var_args &args_;
};


template <>
class variant_type_impl<word> : public variant_type {
public:
  static inline const void *encode(const word &t) {
    return reinterpret_cast<const void*>(t);
  }
  virtual void print_on(const void *data, string modifiers,
      string_stream &stream);
  static variant_type_impl<word> kInstance;
};


template <>
class variant_type_impl<string> : public variant_type {
public:
  static inline const void *encode(const string &t) {
    return static_cast<const void*>(&t);
  }
  virtual void print_on(const void *data, string modifiers,
      string_stream &stream);
  static variant_type_impl<string> kInstance;
};


template <>
class variant_type_impl<format_bundle> : public variant_type {
public:
  static inline const void *encode(const format_bundle &t) {
    return static_cast<const void*>(&t);
  }
  virtual void print_on(const void *data, string modifiers,
      string_stream &stream);
  static variant_type_impl<format_bundle> kInstance;
};


template <>
class variant_type_impl<char> : public variant_type {
public:
  static inline const void *encode(const char &t) {
    return reinterpret_cast<const void*>(static_cast<word>(t));
  }
  virtual void print_on(const void *data, string modifiers,
      string_stream &stream);
  static variant_type_impl<char> kInstance;
};


template <>
class variant_type_impl<bool> : public variant_type {
public:
  static inline const void *encode(const bool &t) {
    return reinterpret_cast<const void*>(static_cast<word>(t));
  }
  virtual void print_on(const void *data, string modifiers,
      string_stream &stream);
  static variant_type_impl<bool> kInstance;
};


} // namespace positron

#endif // _UTILS_STRING
