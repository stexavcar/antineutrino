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

vector<uint8_t> SexpScanner::scan_token() {
  word start = cursor();
  while (is_token_character(current()))
    advance();
  word length = cursor() - start;
  vector<uint8_t> chars = vector<uint8_t>::allocate(arena(), length);
  for (word i = 0; i < length; i++)
    chars[i] = str()[start + i];
  return chars;
}

SexpScanner::Token SexpScanner::next() {
  SexpScanner::Token result;
  code_point cp = current();
  last_string_ = vector<uint8_t>();
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
      } else {
        advance();
        result = tError;
      }
      break;
  }
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

class parse_result {
public:
  parse_result(s_exp *value) : value_(value) { }
  bool is_failure() { return value_ == NULL; }
  s_exp *value() { return value_; }
private:
  s_exp *value_;
};

class SexpParser {
public:
  SexpParser(string input, Arena &arena);
  parse_result parse_exp();
private:
  void advance();
  SexpScanner &scan() { return scan_; }
  Arena &arena() { return arena_; }
  SexpScanner::Token current() { return current_; }
  SexpScanner scan_;
  Arena &arena_;
  SexpScanner::Token current_;
};

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

parse_result SexpParser::parse_exp() {
  switch (current()) {
    case SexpScanner::tString: {
      vector<uint8_t> str = scan().last_string();
      advance();
      return new (arena()) s_string(str);
    }
    case SexpScanner::tLeft: {
      arena_buffer<s_exp*> buffer(arena());
      advance();
      while (current() != SexpScanner::tRight && current() != SexpScanner::tEnd) {
        try parse s_exp *next = parse_exp();
        buffer.append(next);
      }
      if (current() == SexpScanner::tEnd)
        return NULL;
      advance();
      vector<s_exp*> elements = buffer.as_vector();
      // Watch out: this appears to be unsafe, since we're returning
      // a vector backed by a buffer that will be deleted when we
      // return.  It works because it's an arena buffer that doesn't
      // dispose its underlying storage.
      return new (arena()) s_list(elements);
    }
    default:
      return NULL;
  }
}

s_exp *s_exp::read(string input, Arena &arena) {
  SexpParser parser(input, arena);
  try parse s_exp *result = parser.parse_exp();
  return result;
}

} // namespace neutrino
