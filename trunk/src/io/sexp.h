#ifndef _SEXP
#define _SEXP

#include "heap/ref.h"
#include "heap/values.h"
#include "heap/zone.h"
#include "utils/checks.h"

namespace neutrino {
namespace sexp {

enum Type {
  STRING, LIST, NUMBER, SYMBOL
};

class Sexp : public zone::Object {
public:
  virtual ~Sexp() { }
  virtual Type type() = 0;
  virtual bool equals(Sexp &that) = 0;
  string to_string();
  virtual void write_to(string_buffer &buf) = 0;
};

class Atom : public Sexp {
  
};

class String : public Atom {
public:
  String(string str);
  virtual Type type() { return kType; }
  virtual bool equals(Sexp &that);
  virtual void write_to(string_buffer &buf);
  string &str() { return str_; }
  static const Type kType = STRING;
private:
  string str_;
};

class Symbol : public Atom {
public:
  Symbol(int kind, string name) : kind_(kind), name_(name) { }
  virtual Type type() { return kType; }
  virtual bool equals(Sexp &that);
  virtual void write_to(string_buffer &buf);
  int kind() { return kind_; }
  string name() { return name_; }
  static const Type kType = SYMBOL;
private:
  int kind_;
  string name_;
};

class Number : public Atom {
public:
  Number(int32_t value) : value_(value) { }
  virtual Type type() { return kType; }
  virtual bool equals(Sexp &that);
  virtual void write_to(string_buffer &buf);
  int32_t value() { return value_; }
  static const Type kType = NUMBER;
private:
  int32_t value_;
};

class List : public Sexp {
public:
  List();
  List(Sexp &arg0);
  List(Sexp &arg0, Sexp &arg1);
  List(Sexp &arg0, Sexp &arg1, Sexp &arg2);
  List(Sexp &arg0, Sexp &arg1, Sexp &arg2, Sexp &arg3);
  List(uint32_t argc, Sexp *argv[]);
  int tag();
  virtual Type type() { return kType; }
  virtual bool equals(Sexp &that);
  virtual void write_to(string_buffer &buf);
  uint32_t length() { return terms().length(); }
  Sexp &at(uint32_t index) { return operator[](index); }
  Sexp &operator[](uint32_t index) { return *(terms()[index]); }
  static const Type kType = LIST;
private:
  void initialize(uint32_t argc, Sexp *argv[]);
  zone::Vector<Sexp*> &terms() { return *terms_; }
  zone::Vector<Sexp*> *terms_;
};

template <class C>
inline C &cast(Sexp &that) {
  ASSERT_EQ(that.type(), C::kType);
  return static_cast<C&>(that);
}

class SymbolResolver {
public:
  virtual ~SymbolResolver() { }
  virtual int get_kind_for(neutrino::string str) { return -1; }
  virtual neutrino::string get_name_for(int kind) { return NULL; }
};

class Reader {
public:
  static Sexp *read(ref<neutrino::String> source, SymbolResolver &resolver);
private:
  Reader(ref<neutrino::String> source, SymbolResolver &resolver);

  Sexp *read_sexp();
  String *read_string(uint32_t end);
  Symbol *read_symbol();
  Number *read_number();
  List *read_list();
  void skip_spaces();
  uint32_t current();
  void advance();
  bool has_more();

  ref<neutrino::String> source() { return source_; }
  SymbolResolver &resolver() { return resolver_; }
  
  SymbolResolver &resolver_;
  ref<neutrino::String> source_;
  uint32_t cursor_;
};

} // sexp
} // neutrino

#endif // _SEXP
