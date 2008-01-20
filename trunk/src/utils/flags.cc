#include "utils/flags.h"
#include "utils/list-inl.h"
#include "utils/string-inl.h"

namespace neutrino {

AbstractRegisterFlag *AbstractRegisterFlag::first_ = NULL;

AbstractRegisterFlag::AbstractRegisterFlag(string name, uint32_t argc)
    : name_(name), argc_(argc), next_(first_) {
  first_ = this;
}

RegisterFlag<bool>::RegisterFlag(string name, bool *var)
    : AbstractRegisterFlag(name, 0), var_(var) { }

bool FlagParser::compare_options(string a, string b) {
  if (a.length() != b.length()) return false;
  for (uint32_t i = 0; i < a.length(); i++) {
    uint32_t c = a[i];
    if (c == '_') c = '-';
    if (b[i] != c) return false;
  }
  return true;
}

list<string> FlagParser::parse_flags(list<char*> &args, ErrorHandler on_error) {
  uint32_t cursor = 1;
  list_buffer<string> buf;
  while (cursor < args.length()) {
    string str(args[cursor++]);
    if (str.length() < 2 || str[0] != '-' || str[1] != '-') {
      buf.append(str);
    } else {
      string name = str.substring(2);
      AbstractRegisterFlag *option = AbstractRegisterFlag::first_;
      while (option != NULL) {
        if (compare_options(option->name(), name))
          break;
        option = option->next();
      }
      if (option == NULL) {
        string_buffer message;
        message.printf("Unknown option %", str.chars());
        on_error(message.raw_string());
        UNREACHABLE();
      } else {
        uint32_t argc = option->argc();
        if (args.length() < argc + cursor) {
          string_buffer message;
          message.printf("Not enough arguments to %", str.chars());
          on_error(message.raw_string());
        }
        list<char*> option_args = args.sublist(cursor, argc);
        option->process(option_args);
        cursor += argc;
      }
    }
  }
  return buf.to_list();
}

void RegisterFlag<bool>::process(list<char*> args) {
  ASSERT(args.length() == 0);
  *var_ = !*var_;
}

} // neutrino
