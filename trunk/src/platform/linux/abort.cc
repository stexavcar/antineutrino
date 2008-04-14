#include <cxxabi.h>
#include <execinfo.h>
#include <stdlib.h>
#include <stdio.h>

#include "platform/posix/abort.cc"
#include "utils/scoped-ptrs-inl.h"

namespace neutrino {

void Abort::abort(string message) {
  message.println(stderr);
  print_stack_trace(0, "neutrino::");
  ::abort();
}

static void print_stack_trace(void *addr, const char *prefix) {
  const uword kTraceSize = 256;
  void *trace[kTraceSize];
  size_t actual_size = backtrace(trace, kTraceSize);
  own_ptr<char*, ptr_free> lines(backtrace_symbols(trace, actual_size));
  fprintf(stderr, "--- Stack ---\n");
  for (uword i = 0; i < actual_size; i++) {
    own_ptr<char, ptr_free> mangled(strdup((*lines)[i]));
    char *start_paren = index(*mangled, '(');
    if (start_paren != NULL) {
      char *end_paren = index(*mangled, '+');
      if (end_paren != NULL) {
        *end_paren = '\0';
        int status = 0;
        own_ptr<char, ptr_free> demangled(abi::__cxa_demangle(start_paren + 1, 0, 0, &status));
        if (status == 0 && *demangled != NULL) {
          remove_substring(*demangled, prefix);
          fprintf(stderr, "%s\n", *demangled);
          continue;
        }
      }
    }
    fprintf(stderr, "%s\n", (*lines)[i]);
  }
}

} // neutrino
