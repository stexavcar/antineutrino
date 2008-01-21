#include "utils/flags.h"
#include "utils/list-inl.h"
#include "utils/string-inl.h"

namespace neutrino {

AbstractRegisterFlag *AbstractRegisterFlag::first_ = NULL;

AbstractRegisterFlag::AbstractRegisterFlag(string name, int32_t argc)
    : name_(name), argc_(argc), next_(first_) {
  first_ = this;
}

RegisterFlag<bool>::RegisterFlag(string name, bool *var)
    : AbstractRegisterFlag(name, 0), var_(var) { }

RegisterFlag< list<string> >::RegisterFlag(string name, list<string> *var)
    : AbstractRegisterFlag(name, -1), var_(var) { }

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
      bool is_list = (str[str.length() - 1] == '[');
      string name = is_list ? str.substring(2, str.length() - 3) : str.substring(2);
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
      } else if (is_list) {
        if (option->argc() >= 0) {
          string_buffer message;
          message.printf("% is not a list option", str.chars());
          on_error(message.raw_string());
        } else {
          uint32_t start = cursor;
          while (cursor < args.length() && !string::equals(args[cursor], "]"))
            cursor++;
          if (cursor == args.length()) {
            string_buffer message;
            message.printf("Malformed list option %", str.chars());
            on_error(message.raw_string());
          } else {
            list<char*> option_args = args.sublist(start, cursor - start);
            option->process(option_args);
            cursor++;
          }
        }
      } else {
        int32_t argc = option->argc();
        if (argc < 0) {
          string_buffer message;
          message.printf("% is a list option", str.chars());
          on_error(message.raw_string());
        }
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

void RegisterFlag< list<string> >::process(list<char*> args) {
  string *array = new string[args.length()];
  for (uint32_t i = 0; i < args.length(); i++)
    array[i] = args[i];
  *var_ = list<string>(array, args.length());
}

} // neutrino
