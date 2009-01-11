#include "utils/array-inl.h"
#include "utils/buffer-inl.h"
#include "utils/string.h"
#include <cstdio>

namespace positron {

void string_stream::add(char c) {
  buf().append(c);
  if (c == '\n') {
    for (size_t i = 0; i < indent().length(); i++)
      buf().append(indent()[i]);
  }
}

void string_stream::add(string format, const var_args &args) {
  word arg_cursor = 0;
  for (size_t i = 0; i < format.length(); i++) {
    char c = format[i];
    switch (c) {
      case '%': case '@': {
        word index;
        if (c == '%') {
          index = arg_cursor++;
        } else if (i + 1 < format.length()) {
          c = format[++i];
          assert '0' <= c;
          assert c <= '9';
          index = (c - '0');
        } else {
          index = 0;
        }
        const variant &var = args[index];
        string modifiers;
        // If there are modifiers read them into 'modifiers'.
        if (i + 1 < format.length() && format[i + 1] == '{') {
          i++;
          word start = i + 1;
          while (i < format.length() && format[i] != '}')
            i++;
          modifiers = string(&format[start], i - start);
        }
        var.print_on(*this, modifiers);
        break;
      }
      default:
        add(c);
        break;
    }
  }
}

void string_stream::add(string str) {
  for (size_t i = 0; i < str.length(); i++)
    add(str[i]);
}

string string_stream::raw_c_str() {
  buf().ensure_capacity(buf().length() + 1);
  buf()[buf().length()] = '\0';
  return string(buf().start(), buf().length());
}

void variant::print_on(string_stream &stream, string modifiers) const {
  type().print_on(data(), modifiers, stream);
}

variant_type_impl<word> variant_type_impl<word>::kInstance;

static word digit_to_char(word digit) {
  if (digit < 10) return '0' + digit;
  else return 'a' + (digit - 10);
}

void variant_type_impl<word>::print_on(const void *data, string modifiers,
    string_stream &buf) {
  // Configuration parameters
  bool flush_right = true;
  word min_length = 0;
  word pad_char = ' ';
  word radix = 10;

  // Convert the number to a string in a temporary buffer
  const char kTempSize = 24;
  embed_array<char, kTempSize> temp;
  word value = reinterpret_cast<long>(data);
  bool is_negative = (value < 0);
  if (is_negative) value = -value;
  word offset = 0;
  if (value == 0) {
    temp[offset] = '0';
    offset++;
  } else {
    while (value > 0) {
      temp[offset] = digit_to_char(value % radix);
      value = value / radix;
      offset = offset + 1;
    }
  }
  word padding = min_length - (offset + (is_negative ? 1 : 0));
  if (is_negative) buf.add('-');
  if (flush_right) {
    for (word i = 0; i < padding; i++)
      buf.add(pad_char);
  }
  for (word i = offset - 1; i >= 0; i--)
    buf.add(temp[i]);
  if (!flush_right) {
    for (word i = 0; i < padding; i++)
      buf.add(pad_char);
  }
}

variant_type_impl<string> variant_type_impl<string>::kInstance;

void variant_type_impl<string>::print_on(const void *data,
    string modifiers, string_stream &buf) {
  const string &value = *static_cast<const string*>(data);
  bool quote = modifiers.contains('q');
  if (quote) buf.add('"');
  buf.add(value);
  if (quote) buf.add('"');
}

variant_type_impl<char> variant_type_impl<char>::kInstance;

void variant_type_impl<char>::print_on(const void *data,
    string modifiers, string_stream &buf) {
  long value = reinterpret_cast<long>(data);
  buf.add(value);
}

variant_type_impl<format_bundle> variant_type_impl<format_bundle>::kInstance;

void variant_type_impl<format_bundle>::print_on(const void *data,
    string modifiers, string_stream &buf) {
  const format_bundle &value = *static_cast<const format_bundle*>(data);
  buf.add(value.format(), value.args());
}

bool string::operator==(const string &that) const {
  if (length() != that.length()) return false;
  for (size_t i = 0; i < length(); i++) {
    if (this->operator[](i) != that[i])
      return false;
  }
  return true;
}

bool string::contains(char c) {
  for (size_t i = 0; i < length(); i++) {
    if (operator[](i) == c)
      return true;
  }
  return false;
}

} // namespace positron
