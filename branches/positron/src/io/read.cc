#include "io/miniheap-inl.h"
#include "io/read.h"
#include "utils/arena-inl.h"
#include "utils/buffer-inl.h"
#include "utils/vector-inl.h"
#include "plankton/plankton-inl.h"

namespace neutrino {

// ---
// S c a n n e r
// ---

class ReadService {
public:
  ReadService();
  p::Value parse(Message &message);
  ObjectProxyDTable<ReadService> &dtable() { return dtable_; }
private:
  ObjectProxyDTable<ReadService> dtable_;
};

ReadService::ReadService() {
  dtable().add_method("parse", &ReadService::parse);
}

REGISTER_SERVICE(neutrino.io.read, s_exp::create_service);

p::Object s_exp::create_service() {
  static ReadService *service = NULL;
  if (service == NULL)
    service = new ReadService();
  return service->dtable().proxy_for(*service);
}

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
  vector<uint8_t> chars = vector<uint8_t>::allocate(arena_array_allocator<uint8_t>(arena()), length);
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

p::Value ReadService::parse(Message &message) {
  assert message.data() != static_cast<void*>(NULL);
  own_ptr<Factory> factory(new Factory());
  message.data()->acquire_resource(*factory.release());
  return Factory::get_void();
}

// ---
// M a t c h i n g
// ---

bool s_string::match(const pattern &p) {
  return p.match_string(this);
}

bool s_list::match(const pattern &p) {
  return p.match_list(this);
}

bool m_string::match_string(s_string *that) const {
  if (!str().is_empty()) {
    return str() == that->chars();
  } else {
    *str_ptr() = that->chars();
    return true;
  }
}

bool m_list::match_list(s_list *that) const {
  word length = that->length();
  if (length > 0 && !that->get(0)->match(e0_)) return false;
  if (length > 1 && !that->get(1)->match(e1_)) return false;
  if (length > 2 && !that->get(2)->match(e2_)) return false;
  if (length > 3 && !that->get(3)->match(e3_)) return false;
  if (length > 4 && !that->get(4)->match(e4_)) return false;
  if (length > 5 && !that->get(5)->match(e5_)) return false;
  return true;
}

} // namespace neutrino
