#include "utils/vector-inl.h"
#include "utils/buffer-inl.h"
#include "utils/string.h"
#include <cstdio>

namespace neutrino {

void string_stream::add(char c) {
  buf().append(c);
  if (c == '\n') {
    for (word i = 0; i < indent().length(); i++)
      buf().append(indent()[i]);
  }
}

void string_stream::add(string format, const var_args &args) {
  word arg_cursor = 0;
  for (word i = 0; i < format.length(); i++) {
    char c = format[i];
    switch (c) {
      case '%': case '@': {
        word index;
        if (c == '%') {
          index = arg_cursor++;
        } else if (i + 1 < format.length()) {
          c = format[++i];
          if (c == '%' || c == '@') {
            add(c);
            break;
          }
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

string string::substring(word start, word end) {
  if (end < 0) end += length() + 1;
  assert 0 <= start;
  assert start <= length();
  assert 0 <= end;
  assert end <= length();
  assert start <= end;
  return string(this->start() + start, end - start);
}

void string_stream::add(string str) {
  for (word i = 0; i < str.length(); i++)
    add(str[i]);
}

string string_stream::raw_string() {
  buf().ensure_capacity(buf().length() + 1);
  buf()[buf().length()] = '\0';
  return string(buf().start(), buf().length());
}

void variant::print_on(string_stream &stream, string modifiers) const {
  type().print_on(*this, modifiers, stream);
}

variant_type_impl<word> variant_type_impl<word>::kInstance;

static word digit_to_char(word digit) {
  if (digit < 10) return '0' + digit;
  else return 'a' + (digit - 10);
}

static bool is_digit(char c) {
  return '0' <= c && c <= '9';
}

static word char_to_digit(char chr) {
  assert is_digit(chr);
  return chr - '0';
}

void variant_type_impl<word>::print_on(const variant &that, string modifiers,
    string_stream &buf) {
  // Configuration parameters
  bool flush_right = true;
  word min_length = 0;
  word pad_char = ' ';
  word radix = 10;

  for (word pos = 0; pos < modifiers.length();) {
    char current = modifiers[pos];
    if (current == '0') {
      pad_char = '0';
      pos++;
    } else if (is_digit(current)) {
      while (pos < modifiers.length() && is_digit(modifiers[pos])) {
        min_length = 10 * min_length + char_to_digit(modifiers[pos]);
        pos++;
      }
    } else if (current == '-') {
      flush_right = false;
      pos++;
    } else if (current == 'x') {
      radix = 16;
      pos++;
    } else {
      assert false;
    }
  }

  // Convert the number to a string in a temporary buffer
  const char kTempSize = 24;
  embed_vector<char, kTempSize> temp;
  word value = that.data_.u_int;
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

void variant_type_impl<string>::print_on(const variant &that,
    string modifiers, string_stream &buf) {
  string value(that.data_.u_c_str.chars, that.data_.u_c_str.length);
  bool quote = modifiers.contains('q');
  if (quote) buf.add('"');
  buf.add(value);
  if (quote) buf.add('"');
}

variant_type_impl<char> variant_type_impl<char>::kInstance;

void variant_type_impl<char>::print_on(const variant &that,
    string modifiers, string_stream &buf) {
  word value = that.data_.u_int;
  buf.add(value);
}

variant_type_impl<bool> variant_type_impl<bool>::kInstance;

void variant_type_impl<bool>::print_on(const variant &that,
    string modifiers, string_stream &buf) {
  word value = that.data_.u_int;
  buf.add(value ? "true" : "false");
}

variant_type_impl<double> variant_type_impl<double>::kInstance;

void variant_type_impl<double>::print_on(const variant &that,
    string modifiers, string_stream &buf) {
  double value = that.data_.u_double;
  if (modifiers.contains('b')) {
    uint64_t bits = double_bits(value);
    buf.add("[");
    for (word i = 63; i >= 0; i--) {
      if (i == 62 || i == 51) buf.add("|");
      buf.add((bits >> i) & 1 ? "1" : "0");
    }
    buf.add("]");
  } else {
    embed_vector<char, 24> scratch;
    sprintf(scratch.start(), "%f", value);
    buf.add(scratch.start());
  }
}

variant_type_impl<format_bundle> variant_type_impl<format_bundle>::kInstance;

void variant_type_impl<format_bundle>::print_on(const variant &that,
    string modifiers, string_stream &buf) {
  const format_bundle &value = *static_cast<const format_bundle*>(that.data_.u_ptr);
  buf.add(value.format(), value.args());
}

bool string::operator==(const string &that) const {
  if (length() != that.length()) return false;
  for (word i = 0; i < length(); i++) {
    if (this->operator[](i) != that[i])
      return false;
  }
  return true;
}

bool string::operator==(const vector<uint8_t> &that) const {
  if (length() != that.length()) return false;
  for (word i = 0; i < length(); i++) {
    if (this->operator[](i) != that[i])
      return false;
  }
  return true;
}

bool string::contains(char c) {
  for (word i = 0; i < length(); i++) {
    if (operator[](i) == c)
      return true;
  }
  return false;
}

string string::dup(string arg) {
  char *result = new char[arg.length() + 1];
  memcpy(result, arg.start(), arg.length());
  result[arg.length()] = '\0';
  return string(result, arg.length());
}

void string::println(FILE *out) {
  for (word i = 0; i < length(); i++)
    fputc(this->operator[](i), out);
  fputc('\n', out);
  fflush(out);
}

template <>
uword hash<string>(const string &str) {
  uword hash = 0;
  uword rotand = 0;
  for (word i = 0; i < str.length(); i++) {
    uword c = str[i];
    rotand ^= c & ((8 * sizeof(uword)) - 1);
    hash = ((hash << rotand) | (hash >> rotand)) ^ c;
  }
  return hash;
}

} // namespace neutrino