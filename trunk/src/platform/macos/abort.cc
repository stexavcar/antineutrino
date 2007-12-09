#include "platform/posix/abort.cc"

#include <dlfcn.h>
#include <cxxabi.h>
#include <string>

namespace neutrino {

void Abort::abort() {
  print_stack_trace(0, "neutrino::");
  ::abort();
}

/**
 * Removes all occurrences of the given substring from the string.
 */
static void remove_substring(char *str, const char *substr) {
  uint32_t p = 0;
  uint32_t i = 0;
  while (str[p]) {
    uint32_t j;
    for (j = 0; str[p + j] == substr[j] && substr[j]; j++);
    if (substr[j]) {
      str[i] = str[p];
      p++;
      i++;
    } else {
      p += j;
    }
  }
  str[i] = '\0';
}

static void print_stack_trace(void *start, const char *prefix) {
  fprintf(stderr, "--- Stack ---\n");
  void **fp = reinterpret_cast<void**>(__builtin_frame_address(0));
  do {
    void *pc = fp[1];
    Dl_info info;
    if (dladdr(pc, &info)) {
      const char *mangled = info.dli_sname;
      int status = 0;
      char *demangled = abi::__cxa_demangle(mangled, 0, 0, &status);
      if (demangled) {
        remove_substring(demangled, prefix);
        fprintf(stderr, "%s\n", demangled);
        free(demangled);
      } else {
        fprintf(stderr, "%s(...)\n", mangled);
        if (strcmp("main", mangled) == 0)
          break;
      }
    }
    fp = static_cast<void**>(fp[0]);
  } while (fp && fp[1]);
}

} // neutrino
