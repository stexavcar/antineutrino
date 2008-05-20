#include "utils/list-inl.h"
#include "utils/string-inl.h"
#include "platform/stdc-inl.h"
#include "heap/ref-inl.h"

#include <string.h>

using namespace neutrino;

// --- S t r i n g ---

string string::dup(string arg) {
  char *result = new char[arg.length() + 1];
  memcpy(result, arg.chars(), arg.length());
  result[arg.length()] = '\0';
  return string(result, arg.length());
}

bool string::equals(const char* a, const char* b) {
  int i;
  for (i = 0; a[i] != '\0'; i++) {
    if (a[i] != b[i])
      return false;
  }
  return b[i] == '\0';
}

bool string::operator==(string that) {
  if (length() != that.length()) return false;
  for (uword i = 0; i < length(); i++) {
    if (operator[](i) != that[i])
      return false;
  }
  return true;
}

void string::dispose() {
  delete[] chars_;
}

void string::println(FILE *out) {
  if (out == NULL) out = stdout;
  print(out);
  ::fputc('\n', out);
}

void string::print(FILE *out) {
  if (out == NULL) out = stdout;
  for (uword i = 0; i < length(); i++)
    ::fputc(chars_[i], out);
}

// --- S t r i n g   b u f f e r ---

string_buffer::string_buffer(uword capacity)
    : data_(new char[capacity])
    , capacity_(capacity)
    , cursor_(0) { }

string_buffer::~string_buffer() {
  delete[] data_;
}

void string_buffer::append(string str) {
  ensure_capacity(str.length());
  for (uword i = 0; i < str.length(); i++)
    data_[cursor_ + i] = str[i];
  cursor_ += str.length();
}

void string_buffer::append(char c) {
  ensure_capacity(1);
  data_[cursor_++] = c;
}

void string_buffer::clear() {
  cursor_ = 0;
}
  
void string_buffer::ensure_capacity(int required) {
  uword previous_capacity = cursor_ + required;
  if (previous_capacity < capacity_) return;
  uword new_capacity = grow_value(previous_capacity);
  char *new_data = new char[new_capacity];
  for (uword i = 0; i < capacity_; i++)
    new_data[i] = data_[i];
  delete[] data_;
  data_ = new_data;
  capacity_ = new_capacity;
}

string string_buffer::to_string() {
  return string::dup(raw_string());
}

string string_buffer::raw_string() {
  return string(data_, cursor_);
}

void string_buffer::printf(string format, const fmt_elms &args) {
  // Make the params buffer large enough to allow changes before and
  // after the param string
  list_value<char, 16> params;
  uword i = 0;
  uword cursor = 0;
  while (i < format.length()) {
    char c = format[i];
    bool is_positional = false;
    switch (c) {
    // Catch format characters but fall through to the bottom so we
    // can share as much code as possible between them.
    case '$':
      is_positional = true;
      break;
    case '%':
      is_positional = false;
      break;
    default:
      // If we're at a non-format character finish quickly
      append(c);
      i++;
      continue;
    }
    i++;
    const fmt_elm *fmt_elm = NULL;
    if (is_positional) {
      ASSERT(i < format.length());
      // Read the fmt_elm from the specified position in the args
      // array
      c = format[i++];
      ASSERT('0' <= c && c <= '9');
      uword index = c - '0';
      fmt_elm = &args[index];
    } else {
      // Read the next fmt_elm
      fmt_elm = &args[cursor++];
    }
    int offset = -1;
    // If we're looking at a '{' we need to transfer the format
    // parameters between the braces to the params string
    string str_params;
    if (i < format.length() && format[i] == '{') {
      i++;
      offset = 0;
      while (true) {
        // No format parameters should take more then a certain fixed
        // number of characters
        c = format[i++];
        if (c == '}') {
          // Close the params string and stop when we reach the end
          params[offset] = '\0';
          str_params = string(params.start(), offset);
          break;
        } else {
          params[offset++] = c;
        }
      }
    }
    fmt_elm->print_on(*this, (offset == -1) ? string() : str_params);
  }
}

static bool is_digit(uword c) {
  return '0' <= c && c <= '9';
}

static uword digit_to_char(uword digit) {
  if (digit < 10) return '0' + digit;
  else return 'a' + (digit - 10);
}

static uword char_to_digit(uword chr) {
  ASSERT(is_digit(chr));
  return chr - '0';
}

void fmt_elm::print_on(string_buffer &buf, string params) const {
  switch (tag_) {
  case eInt:
    print_int_on(buf, params);
    break;
  case eDouble: {
    UNREACHABLE();
    break;
  }
  case eCStr: {
    const char *value = value_.u_c_str;
    buf.append(value);
    break;
  }
  case eString: {
    const string &str = *(value_.u_string);
    buf.append(str);
    break;
  }
  case eObject: {
    Data *data = value_.u_object;
    data->write_on(buf, Data::UNQUOTED);
    break;
  }
  case eRef: {
    ref<Value> val = *value_.u_ref;
    val->write_on(buf, Data::UNQUOTED);
    break;
  }
  default:
    UNREACHABLE();
    break;
  }
}


void fmt_elm::print_int_on(string_buffer &buf, string params) const {
  // Configuration parameters
  bool flush_right = true;
  uword min_length = 0;
  uword pad_char = ' ';
  uword radix = 10;
  // Process parameter string and set above parameters
  for (uword pos = 0; pos < params.length();) {
    uword current = params[pos];
    if (current == '0') {
      pad_char = '0';
      pos++;
    } else if (is_digit(current)) {
      while (pos < params.length() && is_digit(params[pos])) {
        min_length = 10 * min_length + char_to_digit(params[pos]);
        pos++;
      }
    } else if (current == '-') {
      flush_right = false;
      pos++;
    } else if (current == 'x') {
      radix = 16;
      pos++;
    } else {
      UNREACHABLE();
    }
  }

  // Convert the number to a string in a temporary buffer
  const char kTempSize = 24;
  char temp_buffer[kTempSize];
  list<char> temp(temp_buffer, kTempSize);
  word value = value_.u_int;
  bool is_negative = (value < 0);
  if (is_negative) value = -value;
  uword offset = 0;
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
  if (is_negative) buf.append("-");
  if (flush_right) {
    for (word i = 0; i < padding; i++)
      buf.append(pad_char);
  }
  for (word i = offset - 1; i >= 0; i--)
    buf.append(temp[i]);
  if (!flush_right) {
    for (word i = 0; i < padding; i++)
      buf.append(pad_char);
  }
}
