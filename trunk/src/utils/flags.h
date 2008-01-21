#ifndef _UTILS_FLAGS
#define _UTILS_FLAGS

#include "utils/list.h"
#include "utils/string.h"

namespace neutrino {

#define REGISTER_FLAG(type, Class, name)                             \
  static RegisterFlag<type> register_##name(#name, &Class::name)

typedef void (ErrorHandler)(string message);

class FlagParser {
private:
  static bool compare_options(string a, string b);
public:
  static list<string> parse_flags(list<char*> &args, ErrorHandler on_error);
};

class AbstractRegisterFlag {
public:
  AbstractRegisterFlag(string name, int32_t argc);
  virtual void process(list<char*> args) = 0;
private:
  string name() { return name_; }
  int32_t argc() { return argc_; }
  AbstractRegisterFlag *next() { return next_; }
  
  string name_;
  int32_t argc_;
  AbstractRegisterFlag *next_;

  static AbstractRegisterFlag *first_;
  
  friend class FlagParser;
};

template <typename T>
class RegisterFlag : public AbstractRegisterFlag { };

template <>
class RegisterFlag<bool> : public AbstractRegisterFlag {
public:
  RegisterFlag(string name, bool *var);
  virtual void process(list<char*> args);
private:
  bool *var_;
};

template <>
class RegisterFlag< list<string> > : public AbstractRegisterFlag {
public:
  RegisterFlag(string name, list<string> *var);
  virtual void process(list<char*> args);
private:
  list<string> *var_;
};

}

#endif // _UTILS_FLAGS
