#include "platform/posix/abort.cc"

#include <stdlib.h>
#include <dlfcn.h>
#include <cxxabi.h>

namespace neutrino {

void Abort::abort() {
  ::abort();
}

void print_stack_trace(void *ptr, const char *prefix) {
  fprintf(stderr, "--- Stack ---\n");
  ucontext_t *ucontext = reinterpret_cast<ucontext_t*>(ptr);
  void **fp = reinterpret_cast<void**>(ucontext->uc_mcontext.gregs[REG_EBP]);
  void *pc = reinterpret_cast<void*>(ucontext->uc_mcontext.gregs[REG_EIP]);
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
      } else if (mangled) {
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
