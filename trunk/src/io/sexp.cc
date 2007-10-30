#include "heap/ref-inl.h"
#include "heap/values-inl.h"
#include "heap/zone-inl.h"
#include "io/sexp.h"
#include "utils/checks.h"
#include "utils/list-inl.h"
#include "utils/string.h"

#include <string.h>

namespace n = neutrino;
using namespace neutrino::sexp;

// --- S e x p s ---

String::String(string str) {
  const char *data = str.chars();
  uint32_t length = str.length();
  size_t size = (length + 1) * sizeof(char);
  int8_t *memory = zone::Zone::current().allocate(size);
  memcpy(memory, data, length * sizeof(char));
  char *zone_data = reinterpret_cast<char*>(memory);
  zone_data[length] = '\0';
  str_ = string(zone_data, length);
}

bool String::equals(Sexp &sexp) {
  if (sexp.type() != STRING) return false;
  String &that = cast<String>(sexp);
  return this->str() == that.str();
}

bool Number::equals(Sexp &sexp) {
  if (sexp.type() != NUMBER) return false;
  Number &that = cast<Number>(sexp);
  return this->value() == that.value();
}

bool Symbol::equals(Sexp &sexp) {
  if (sexp.type() != SYMBOL) return false;
  Symbol &that = cast<Symbol>(sexp);
  return this->kind() == that.kind();
}

bool List::equals(Sexp &sexp) {
  if (sexp.type() != LIST) return false;
  List &that = cast<List>(sexp);
  if (this->length() != that.length()) return false;
  for (uint32_t i = 0; i < this->length(); i++) {
    if (!this->at(i).equals(that[i]))
      return false;
  }
  return true;
}

int List::tag() {
  ASSERT(length() > 0);
  return cast<Symbol>(this->at(0)).kind();
}

n::string Sexp::to_string() {
  string_buffer buf;
  write_to(buf);
  return buf.to_string();
}

void List::write_to(string_buffer &buf) {
  buf.append('[');
  bool first = true;
  for (uint32_t i = 0; i < length(); i++) {
    if (first) first = false;
    else buf.append(' ');
    this->at(i).write_to(buf);
  }
  buf.append(']');
}

void String::write_to(string_buffer &buf) {
  buf.printf("'%'", str().chars());
}

void Symbol::write_to(string_buffer &buf) {
  if (name().is_empty()) {
    buf.append("#(null)");
  } else {
    buf.printf("#%", name().chars());
  }
}

void Number::write_to(string_buffer &buf) {
  buf.printf("%", value());
}

List::List() {
  initialize(0, NULL);
}

List::List(Sexp &arg0) {
  Sexp *args[1] = { &arg0 };
  initialize(1, args);
}

List::List(Sexp &arg0, Sexp &arg1) {
  Sexp *args[2] = { &arg0, &arg1 };
  initialize(2, args);
}

List::List(Sexp &arg0, Sexp &arg1, Sexp &arg2) {
  Sexp *args[3] = { &arg0, &arg1, &arg2 };
  initialize(3, args);
}

List::List(Sexp &arg0, Sexp &arg1, Sexp &arg2, Sexp &arg3) {
  Sexp *args[4] = { &arg0, &arg1, &arg2, &arg3 };
  initialize(4, args);
}

List::List(uint32_t argc, Sexp *argv[]) {
  initialize(argc, argv);
}

void List::initialize(uint32_t argc, Sexp *argv[]) {
  terms_ = new n::zone::Vector<Sexp*>(argc);
  for (uint32_t i = 0; i < argc; i++)
    terms_->at(i) = argv[i];
}

// --- P a r s e r ---

Reader::Reader(ref<neutrino::String> source, SymbolResolver &resolver)
    : resolver_(resolver), source_(source), cursor_(0) { }

uint32_t Reader::current() {
  return source()->at(cursor_);
}

void Reader::advance() {
  if (has_more())
    cursor_++;
}

bool Reader::has_more() {
  return cursor_ < source()->length();
}

Sexp *Reader::read(ref<neutrino::String> source,
    SymbolResolver &resolver) {
  Reader reader(source, resolver);
  reader.skip_spaces();
  Sexp *result = reader.read_sexp();
  ASSERT(!reader.has_more());
  return result;
}

static bool is_whitespace(uint32_t ch) {
  switch (ch) {
  case ' ': case '\t': case '\n': case '\r': case '\f':
    return true;
  default:
    return false;
  }
}

static bool is_identifier_start(uint32_t ch) {
  return 'a' <= ch && ch <= 'z'
      || 'A' <= ch && ch <= 'Z'
      || ch == '_';
}

static bool is_digit(uint32_t ch) {
  return '0' <= ch && ch <= '9';
}

static uint32_t digit_value(uint32_t ch) {
  ASSERT(is_digit(ch));
  return ch - '0';
}

static bool is_identifier_part(uint32_t ch) {
  return is_identifier_start(ch) || is_digit(ch);
}

void Reader::skip_spaces() {
  while (has_more() && is_whitespace(current()))
    advance();
}

String *Reader::read_string(uint32_t end) {
  ASSERT(current() == end);
  advance();
  string_buffer buf;
  while (has_more() && current() != end) {
    buf.append(current());
    advance();
  }
  ASSERT(has_more());
  advance();
  skip_spaces();
  return new String(buf.raw_string());
}

Symbol *Reader::read_symbol() {
  string_buffer buf;
  while  (has_more() && is_identifier_part(current())) {
    buf.append(current());
    advance();
  }
  skip_spaces();
  int kind = resolver().get_kind_for(buf.raw_string());
  string name = resolver().get_name_for(kind);
  return new Symbol(kind, name);
}

Number *Reader::read_number() {
  int32_t value = 0;
  while (has_more() && is_digit(current())) {
    value = 10 * value + digit_value(current());
    advance();
  }
  skip_spaces();
  return new Number(value);
}

List *Reader::read_list() {
  ASSERT(current() == '(');
  advance();
  skip_spaces();
  list_buffer<Sexp*> exprs;
  while (has_more() && current() != ')') {
    Sexp *next = read_sexp();
    ASSERT(next != NULL);
    exprs.append(next);
  }
  ASSERT(current() == ')');
  advance();
  skip_spaces();
  return new List(exprs.length(), exprs.data());
}

Sexp *Reader::read_sexp() {
  switch (current()) {
  case '(':
    return read_list();
  case '"': case '\'':
    return read_string(current());
  default:
    if (is_identifier_start(current())) {
      return read_symbol();
    } else if (is_digit(current())) {
      return read_number();
    } else {
      return NULL;
    }
  }
}
