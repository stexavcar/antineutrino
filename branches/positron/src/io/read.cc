#include "io/read.h"
#include "utils/arena-inl.h"
#include "utils/buffer-inl.h"
#include "utils/vector-inl.h"

namespace neutrino {

// ---
// S c a n n e r
// ---

SexpScanner::SexpScanner(string str, Arena &arena)
  : arena_(arena)
  , str_(str)
  , pos_(0) {
  advance();
  skip_whitespace();
}

static bool is_whitespace(code_point cp) {
  switch (cp) {
    case ' ': case '\t': case '\n': case '\r': case '\f': case '\v':
      return true;
    default:
      return false;
  }
}

static bool is_alphabetic(code_point cp) {
  return ('a' <= cp && cp <= 'z')
      || ('A' <= cp && cp <= 'Z');
}

static bool is_pseudo_alphabetic(code_point cp) {
  switch (cp) {
    case '-': case '.': case '_': case ':': case '*': case '+':
    case '=':
      return true;
    default:
      return false;
  }
}

static bool is_numeric(code_point cp) {
  return ('0' <= cp && cp <= '9');
}

static bool is_token_character(code_point cp) {
  return is_alphabetic(cp) || is_numeric(cp) || is_pseudo_alphabetic(cp);
}

void SexpScanner::advance() {
  if (pos_ < str().length()) {
    current_ = str()[pos_];
    pos_++;
  } else if (pos_ == str().length()) {
    current_ = '\0';
    pos_++;
  }
}

string SexpScanner::scan_token() {
  word start = cursor();
  while (is_token_character(current()))
    advance();
  word length = cursor() - start;
  arena_array_allocator<char> allocator(arena());
  vector<char> chars = vector<char>::allocate(allocator, length);
  for (word i = 0; i < length; i++)
    chars[i] = str()[start + i];
  return string(chars.start(), length);
}

word SexpScanner::scan_number() {
  word value = 0;
  while (is_numeric(current())) {
    value = (10 * value) + (current() - '0');
    advance();
  }
  return value;
}

SexpScanner::Token SexpScanner::next() {
  SexpScanner::Token result;
  code_point cp = current();
  last_string_ = string();
  location_.start = cursor();
  switch (cp) {
    case '\0':
      result = tEnd;
      break;
    case '(':
      advance();
      result = tLeft;
      break;
    case ')':
      advance();
      result = tRight;
      break;
    default:
      if (is_alphabetic(cp)) {
        last_string_ = scan_token();
        result = tString;
      } else if (is_numeric(cp)) {
        last_number_ = scan_number();
        result = tNumber;
      } else {
        advance();
        result = tError;
      }
      break;
  }
  location_.end = cursor() - 1;
  skip_whitespace();
  return result;
}

void SexpScanner::skip_whitespace() {
  while (is_whitespace(current()))
      advance();
}

// ---
// P a r s e r
// ---

SexpParser::SexpParser(string input, Arena &arena)
  : scan_(input, arena)
  , arena_(arena) {
  advance();
}

void SexpParser::advance() {
  current_ = scan().next();
}

#define pTryParse(Type, name, expr)                                  \
  Type *name;                                                        \
  do {                                                               \
    parse_result __result__ = expr;                                  \
    if (__result__.is_failure())                                     \
      return NULL;                                                   \
    name = __result__.value();                                       \
  } while (false)

s_exp *SexpParser::record_error() {
  error_location_ = scan().location();
  return NULL;
}

parse_result SexpParser::parse_exp() {
  switch (current()) {
    case SexpScanner::tString: {
      string str = scan().last_string();
      advance();
      return new (arena()) s_string(str);
    }
    case SexpScanner::tNumber: {
      word val = scan().last_number();
      advance();
      return new (arena()) s_number(val);
    }
    case SexpScanner::tLeft: {
      arena_buffer<s_exp*> buffer(arena());
      advance();
      while (current() != SexpScanner::tRight && current() != SexpScanner::tEnd) {
        try parse s_exp *next = parse_exp();
        buffer.append(next);
      }
      if (current() == SexpScanner::tEnd)
        return record_error();
      advance();
      vector<s_exp*> elements = buffer.as_vector();
      // Watch out: this appears to be unsafe, since we're returning
      // a vector backed by a buffer that will be deleted when we
      // return.  It works because it's an arena buffer that doesn't
      // dispose its underlying storage.
      return new (arena()) s_list(elements);
    }
    default:
      return record_error();
  }
}

s_exp *SexpParser::read() {
  try parse s_exp *result = parse_exp();
  return result;
}

} // namespace neutrino
