#ifndef _UTILS_STRING
#define _UTILS_STRING


#include <cstring>
#include <cstdio>


#include "utils/vector.h"
#include "utils/buffer.h"
#include "utils/hash-map.h"


namespace neutrino {


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
  bool operator==(const vector<uint8_t> &that) const;
  bool contains(char c);
  word length() const { return length_; }
  const char *start() const { return chars_; }
  const char *end() const { return chars_ + length_; }
  const char &operator[](word i) const { return chars_[i]; }
  static string dup(string arg);
  vector<const char> chars() { return vector<const char>(chars_, length_); }
  bool is_empty() const { return chars_ == NULL; }
  void println(FILE *out = stdout);
private:
  const char *chars_;
  word length_;
};


template <> uword hash<string>(const string &str);


class variant {
public:
  template <typename T>
  inline variant(const T &t) : type_(NULL) {
    encode_variant(*this, static_cast<typename coerce<T>::type>(t));
  }
  void print_on(string_stream &stream, string modifiers) const;
  variant_type &type() const { return *type_; }
  variant_type *type_;
  union {
    const void *u_ptr;
    word u_int;
    struct {
      word length;
      const char *chars;
    } u_c_str;
    struct {
      void *first;
      void *second;
    } u_pair;
  } data_;
};


class variant_type {
public:
  virtual void print_on(const variant &that, string modifiers,
      string_stream &stream) = 0;
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
  virtual void print_on(const variant &that, string modifiers,
      string_stream &stream);
  static variant_type_impl<word> kInstance;
};


static inline void encode_variant(variant &that, word value) {
  that.type_ = &variant_type_impl<word>::kInstance;
  that.data_.u_int = value;
}


template <>
class variant_type_impl<string> : public variant_type {
public:
  virtual void print_on(const variant &that, string modifiers,
      string_stream &stream);
  static variant_type_impl<string> kInstance;
};


static inline void encode_variant(variant &that, string str) {
  that.type_ = &variant_type_impl<string>::kInstance;
  that.data_.u_c_str.length = str.length();
  that.data_.u_c_str.chars = str.start();
}


template <>
class variant_type_impl<format_bundle> : public variant_type {
public:
  virtual void print_on(const variant &that, string modifiers,
      string_stream &stream);
  static variant_type_impl<format_bundle> kInstance;
};


static inline void encode_variant(variant &that, const format_bundle &bundle) {
  that.type_ = &variant_type_impl<format_bundle>::kInstance;
  that.data_.u_ptr = static_cast<const void*>(&bundle);
}


template <>
class variant_type_impl<char> : public variant_type {
public:
  virtual void print_on(const variant &that, string modifiers,
      string_stream &stream);
  static variant_type_impl<char> kInstance;
};


static inline void encode_variant(variant &that, char value) {
  that.type_ = &variant_type_impl<char>::kInstance;
  that.data_.u_int = value;
}


template <>
class variant_type_impl<bool> : public variant_type {
public:
  virtual void print_on(const variant &that, string modifiers,
      string_stream &stream);
  static variant_type_impl<bool> kInstance;
};


static inline void encode_variant(variant &that, bool value) {
  that.type_ = &variant_type_impl<bool>::kInstance;
  that.data_.u_int = value;
}


} // namespace neutrino

#endif // _UTILS_STRING
