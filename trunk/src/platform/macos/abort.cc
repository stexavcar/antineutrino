#include "platform/posix/abort.cc"

#include <dlfcn.h>
#include <cxxabi.h>
#include <string>

namespace neutrino {

void Abort::abort(string message) {
  message.println(stderr);
  print_stack_trace(0, "neutrino::");
  ::abort();
}

static void print_stack_trace(void *ptr, const char *prefix) {
  fprintf(stderr, "--- Stack ---\n");
  void *pc = reinterpret_cast<void*>(__builtin_return_address(0));
  void **fp = reinterpret_cast<void**>(__builtin_frame_address(1));
  while (fp && pc) {
    Dl_info info;
    if (dladdr(pc, &info)) {
      const char *mangled = info.dli_sname;
      int status = 0;
      char *demangled = abi::__cxa_demangle(mangled, 0, 0, &status);
      if (status == 0 && demangled) {
        remove_substring(demangled, prefix);
        fprintf(stderr, "%s\n", demangled);
        free(demangled);
      } else {
        fprintf(stderr, "%s(...)\n", mangled);
        if (strcmp("main", mangled) == 0)
          break;
      }
    }
    pc = fp[1];
    fp = static_cast<void**>(fp[0]);
  }
}

} // neutrino
