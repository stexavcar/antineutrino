#ifndef _UTILS_FLAGS
#define _UTILS_FLAGS

#include "utils/buffer.h"
#include "utils/string.h"

namespace neutrino {

#define REGISTER_FLAG(name, init) do {                               \
  name##_ = init;                                                    \
  register_flag(#name, &name##_);                                    \
} while (false)

class FlagParser {
private:
  static bool compare_options(string a, string b);
public:
  static vector<string> parse_flags(vector<char*> &args);
};

class AbstractFlag {
public:
  AbstractFlag(string name, word argc);
  virtual void process(vector<const char*> args) = 0;
private:
  string name() { return name_; }
  word argc() { return argc_; }

  string name_;
  word argc_;

  friend class OptionCollection;
};

template <typename T>
class Flag : public AbstractFlag { };

template <>
class Flag<bool> : public AbstractFlag {
public:
  Flag(string name, bool *var);
  virtual void process(vector<const char*> args);
private:
  bool *var_;
};

template <>
class Flag< vector<string> > : public AbstractFlag {
public:
  Flag(string name, vector<string> *var);
  virtual void process(vector<const char*> args);
private:
  vector<string> *var_;
};

template <>
class Flag<word> : public AbstractFlag {
public:
  Flag(string name, word *var);
  virtual void process(vector<const char*> args);
private:
  word *var_;
};

class OptionCollection {
public:
  vector<string> parse(vector<const char*> args);
protected:
  OptionCollection();
  ~OptionCollection();
  template <typename T>
  void register_flag(string name, T *target) {
    flags_.append(new Flag<T>(name, target));
  }
  void inherit(OptionCollection &other);
private:
  vector<AbstractFlag*> flags() { return flags_.as_vector(); }
  own_buffer<AbstractFlag> flags_;
};

}

#endif // _UTILS_FLAGS
