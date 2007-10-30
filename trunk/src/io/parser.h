#ifndef _IO_PARSER
#define _IO_PARSER

#include "io/sexp.h"
#include "runtime/runtime.h"

namespace neutrino {

/**
 * The symbol resolver used when parsing programs to convert source
 * symbols into syntax tree tags.
 */
class ProgramSymbolResolver : public sexp::SymbolResolver {
public:
  virtual int get_kind_for(string str);
  virtual string get_name_for(int kind);
};

class Parser {
public:
  Parser(Runtime &runtime);
  void load(sexp::List &program);
  ref<Value> parse_value(sexp::Sexp &expr);
  ref<Code> parse_code(sexp::List &expr);
  ref<Tuple> parse_literals(sexp::List &expr);
  ref<Value> parse_value(sexp::List &expr);
  ref<SyntaxTree> parse_expression(sexp::List &expr);
  Runtime &runtime() { return runtime_; }
  Factory &factory() { return factory_; }
private:
  Runtime &runtime_;
  Factory &factory_;
};

}

#endif // _IO_PARSER
