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

static p::Object create_service() {
  static ReadService *service = NULL;
  if (service == NULL)
    service = new ReadService();
  return service->dtable().proxy_for(*service);
}

REGISTER_SERVICE(neutrino.io.read, create_service);

SexpScanner::SexpScanner(p::String str, Arena &arena)
  : arena_(arena)
  , str_(str)
  , cursor_(-1)
  , at_end_(false) {
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
  if (at_end_) return;
  current_ = str()[++cursor_];
  if (current_ == '\0')
    at_end_ = true;
}

string SexpScanner::scan_token() {
  word start = cursor();
  while (is_token_character(current()))
    advance();
  word length = cursor() - start;
  vector<char> chars = vector<char>::allocate(arena_array_allocator<char>(arena()), length);
  for (word i = 0; i < length; i++)
    chars[i] = str()[start + i];
  return string(chars.start(), chars.length());
}

SexpScanner::Token SexpScanner::next() {
  SexpScanner::Token result;
  code_point cp = current();
  last_string_ = string();
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
  parse_result(p::Value value) : value_(value) { }
  bool is_failure() { return value().is_empty(); }
  p::Value value() { return value_; }
private:
  p::Value value_;
};

class SexpParser {
public:
  SexpParser(p::String input, Factory &factory, Arena &arena);
  parse_result parse_exp();
private:
  void advance();
  SexpScanner &scan() { return scan_; }
  Factory &factory() { return factory_; }
  Arena &arena() { return arena_; }
  SexpScanner::Token current() { return current_; }
  SexpScanner scan_;
  Factory &factory_;
  Arena &arena_;
  SexpScanner::Token current_;
};

SexpParser::SexpParser(p::String input, Factory &factory, Arena &arena)
  : scan_(input, arena)
  , factory_(factory)
  , arena_(arena) {
  advance();
}

void SexpParser::advance() {
  current_ = scan().next();
}

#define pTryParse(Type, name, expr)                                  \
  Type name;                                                         \
  do {                                                               \
    parse_result __result__ = expr;                                  \
    if (__result__.is_failure())                                     \
      return p::Value();                                             \
    name = __result__.value();                                       \
  } while (false)

parse_result SexpParser::parse_exp() {
  switch (current()) {
    case SexpScanner::tString: {
      string str = scan().last_string();
      advance();
      return factory().new_string(str);
    }
    case SexpScanner::tLeft: {
      arena_buffer<p::Value> buffer(arena());
      advance();
      while (current() != SexpScanner::tRight && current() != SexpScanner::tEnd) {
        try parse p::Value ~ next = parse_exp();
        buffer.append(next);
      }
      if (current() == SexpScanner::tEnd)
        return p::Value();
      advance();
      MutableArray elements = factory().new_array(buffer.length());
      for (word i = 0; i < buffer.length(); i++)
        elements.set(i, buffer[i]);
      return elements;
    }
    default:
      return p::Value();
  }
}

p::Value ReadService::parse(Message &message) {
  assert message.data() != static_cast<void*>(NULL);
  assert message.args().length() == 1;
  p::String str = cast<p::String>(message.args()[0]);
  own_ptr<Factory> factory(new Factory());
  Arena arena;
  SexpParser parser(str, *(*factory), arena);
  try parse p::Value ~ result = parser.parse_exp();
  message.data()->acquire_resource(*factory.release());
  return result;
}

} // namespace neutrino
