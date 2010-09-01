#include "utils/buffer-inl.h"
#include "utils/flags.h"
#include "utils/log.h"
#include "utils/string-inl.h"

namespace neutrino {

OptionCollection::OptionCollection() { }
OptionCollection::~OptionCollection() { }

AbstractFlag::AbstractFlag(string name, word argc)
    : name_(name), argc_(argc) { }

Flag<bool>::Flag(string name, bool *var)
    : AbstractFlag(name, 0), var_(var) { }

Flag< vector<string> >::Flag(string name, vector<string> *var)
    : AbstractFlag(name, -1), var_(var) { }

Flag<word>::Flag(string name, word *var)
    : AbstractFlag(name, 1), var_(var) { }

static bool compare_flag_names(string a, string b) {
  if (a.length() != b.length()) return false;
  for (word i = 0; i < a.length(); i++) {
    word c = a[i];
    if (c == '_') c = '-';
    if (b[i] != c) return false;
  }
  return true;
}

vector<string> OptionCollection::parse(vector<const char*> args) {
  word cursor = 1;
  buffer<string> buf;
  while (cursor < args.length()) {
    string str(args[cursor++]);
    if (str.length() < 2 || str[0] != '-' || str[1] != '-') {
      buf.append(str);
    } else {
      bool is_list = (str[str.length() - 1] == '[');
      string name = is_list ? str.substring(2, -2) : str.substring(2);
      AbstractFlag *flag = NULL;
      for (word own = 0; flag == NULL && own < 2; own++) {
        vector<AbstractFlag*> flags = own ? own_flags() : inherited_flags();
        for (word i = 0; i < flags.length(); i++) {
          if (compare_flag_names(flags[i]->name(), name)) {
            flag = flags[i];
            break;
          }
        }
      }
      if (flag == NULL) {
        LOG().error("Unknown option %", vargs(str));
        return vector<string>();
      } else if (is_list) {
        if (flag->argc() >= 0) {
          LOG().error("% is not a list option", vargs(str));
          return vector<string>();
        } else {
          word start = cursor;
          while (cursor < args.length() && string("]") != args[cursor])
            cursor++;
          if (cursor == args.length()) {
            LOG().error("Malformed list option %", vargs(str));
            return vector<string>();
          } else {
            vector<const char*> option_args = args.subvector(start, cursor);
            flag->process(option_args);
            cursor++;
          }
        }
      } else {
        word argc = flag->argc();
        if (argc < 0) {
          LOG().error("% is a list option", vargs(str));
          return vector<string>();
        }
        if (args.length() < argc + cursor) {
          LOG().error("Not enough arguments to %", vargs(str));
          return vector<string>();
        }
        vector<const char*> option_args = args.subvector(cursor, cursor + argc);
        flag->process(option_args);
        cursor += argc;
      }
    }
  }
  return buf.as_vector().dup();
}

void OptionCollection::inherit(OptionCollection &other) {
  for (word own = 0; own < 2; own++) {
    vector<AbstractFlag*> vect = own
        ? other.own_flags()
        : other.inherited_flags();
    for (word i = 0; i < vect.length(); i++)
      inherited_flags_.append(vect[i]);
  }
}

void Flag<bool>::process(vector<const char*> args) {
  assert args.length() == 0;
  *var_ = !*var_;
}

void Flag<word>::process(vector<const char*> args) {
  assert args.length() == 1;
  *var_ = atoi(args[0]);
}

void Flag< vector<string> >::process(vector<const char*> args) {
  vector<string> result = vector<string>::allocate(args.length());
  for (word i = 0; i < args.length(); i++)
    result[i] = args[i];
  *var_ = result;
}

} // namespace neutrino
