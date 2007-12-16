#include "utils/string-inl.h"

#include <string.h>

using namespace neutrino;

#ifdef MSVC
#define snprintf sprintf_s
#endif

// --- S t r i n g ---

uint32_t string::length(const char* chars) {
  int result = 0;
  while (chars[result] != '\0')
    result++;
  return result;
}

string string::dup(string arg) {
  char *result = new char[arg.length() + 1];
  memcpy(result, arg.chars(), arg.length() + 1);
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
  for (uint32_t i = 0; i < length(); i++) {
    if (operator[](i) != that[i])
      return false;
  }
  return true;
}

void string::dispose() {
  delete[] chars_;
}

void string::println() {
  printf("%s\n", chars());
}

// --- S t r i n g   b u f f e r ---

string_buffer::string_buffer(uint32_t capacity)
    : data_(new char[capacity])
    , capacity_(capacity)
    , cursor_(0) { }

string_buffer::~string_buffer() {
  delete[] data_;
}

void string_buffer::append(string str) {
  ensure_capacity(str.length());
  for (uint32_t i = 0; i < str.length(); i++)
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
  uint32_t previous_capacity = cursor_ + required;
  if (previous_capacity < capacity_) return;
  uint32_t new_capacity = grow_value(previous_capacity);
  char *new_data = new char[new_capacity];
  for (uint32_t i = 0; i < capacity_; i++)
    new_data[i] = data_[i];
  delete[] data_;
  data_ = new_data;
  capacity_ = new_capacity;
}

string string_buffer::to_string() {
  int length = cursor_;
  char *chars = new char[length + 1];
  for (int i = 0; i < length; i++)
    chars[i] = data_[i];
  chars[length] = '\0';
  return string(chars, length);
}

string string_buffer::raw_string() {
  return string(data_, cursor_);
}

void string_buffer::printf(string format, element arg1) {
  const int argc = 1;
  element argv[argc] = { arg1 };
  printf(format, argc, argv);
}

void string_buffer::printf(string format, element arg1,
    element arg2) {
  const int argc = 2;
  element argv[argc] = { arg1, arg2 };
  printf(format, argc, argv);
}

void string_buffer::printf(string format, element arg1,
    element arg2, element arg3) {
  const int argc = 3;
  element argv[argc] = { arg1, arg2, arg3 };
  printf(format, argc, argv);
}

void string_buffer::printf(string format, element arg1,
    element arg2, element arg3, element arg4) {
  const int argc = 4;
  element argv[argc] = { arg1, arg2, arg3, arg4 };
  printf(format, argc, argv);
}

void string_buffer::printf(string format, uint32_t argc, element argv[]) {
  const int kMaxParamsLength = 16;
  const int kPrefixLength = 2;
  const int kSuffixLength = 2;
  // Make the params buffer large enough to allow changes before and
  // after the param string
  char params_buffer[kMaxParamsLength + kPrefixLength + kSuffixLength];
  char *params = params_buffer + kPrefixLength;
  uint32_t i = 0;
  uint32_t cursor = 0;
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
    element *element;
    if (is_positional) {
      ASSERT(i < format.length());
      // Read the element from the specified position in the args
      // array
      c = format[i++];
      ASSERT('0' <= c && c <= '9');
      uint32_t index = c - '0';
      ASSERT(index < argc);
      element = &argv[index];
    } else {
      // Read the next element
      ASSERT(cursor < argc);
      element = &argv[cursor++];
    }
    int offset = -1;
    // If we're looking at a '{' we need to transfer the format
    // parameters between the braces to the params string
    if (i < format.length() && format[i] == '{') {
      i++;
      offset = 0;
      while (true) {
        // No format parameters should take more then a certain fixed
        // number of characters
        ASSERT(i < format.length());
        ASSERT(offset < kMaxParamsLength);
        c = format[i++];
        if (c == '}') {
          // Close the params string and stop when we reach the end
          params[offset] = '\0';
          break;
        } else {
          params[offset++] = c;
        }
      }
    }
    element->print_on(*this, (offset == -1) ? NULL : params, offset);
  }
}

void string_buffer::element::print_on(string_buffer &buf, char *params,
    int offset) {
  switch (tag_) {
  case INT_TAG: {
    int value = value_.u_int;
    const char kTempSize = 24;
    char temp[kTempSize];
    if (params) {
      // Use the space before and after the string in the params array
      // to create a native printf format string.
      params[-1] = '%';
      params[offset] = 'i';
      params[offset + 1] = '\0';
      snprintf(temp, kTempSize, params - 1, value);
    } else {
      snprintf(temp, kTempSize, "%i", value);
    }
    buf.append(temp);
    break;
  }
  case DOUBLE_TAG: {
    double value = value_.u_double;
    const char kTempSize = 24;
    char temp[kTempSize];
    if (params) {
      // Use the space before and after the string in the params array
      // to create a native printf format string.
      params[-1] = '%';
      params[offset] = 'g';
      params[offset + 1] = '\0';
      snprintf(temp, kTempSize, params - 1, value);
    } else {
      snprintf(temp, kTempSize, "%g", value);
    }
    buf.append(temp);
    break;
  }
  case STRING_TAG: {
    const char *value = value_.u_string;
    buf.append(value);
    break;
  }
  default:
    UNREACHABLE();
    break;
  }
}
