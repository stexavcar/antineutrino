#include "platform/abort.h"
#include "utils/string-inl.h"
#include "utils/smart-ptrs-inl.h"

#include <cstdio>
#include <cstdlib>
#include <signal.h>
#include <errno.h>
#include <dlfcn.h>
#include <cxxabi.h>
#include <string>

namespace neutrino {

void Abort::abort(string format, const var_args &vars) {
  string_stream buf;
  buf.add(format, vars);
  ::fprintf(stderr, "%s", buf.raw_c_str().start());
  ::abort();
}

static bool install_handler(int signum, void (*handler)(int, siginfo_t*, void*)) {
  struct sigaction action;
  ::memset(&action, 0, sizeof(action));
  action.sa_sigaction = handler;
  action.sa_flags = SA_SIGINFO;
  return ::sigaction(signum, &action, NULL) >= 0;
}

#define eSignalCodes(VISIT)                                          \
  VISIT(0,       SI_USER,     "user signal")                         \
  VISIT(SIGILL,  ILL_ILLOPC,  "illegal opcode")                      \
  VISIT(SIGILL,  ILL_ILLOPN,  "illegal operand")                     \
  VISIT(SIGILL,  ILL_ILLADR,  "illegal addressing mode")             \
  VISIT(SIGILL,  ILL_ILLTRP,  "illegal trap")                        \
  VISIT(SIGILL,  ILL_PRVOPC,  "privileged opcode")                   \
  VISIT(SIGILL,  ILL_PRVREG,  "privileged register")                 \
  VISIT(SIGILL,  ILL_COPROC,  "coprocessor error")                   \
  VISIT(SIGILL,  ILL_BADSTK,  "internal stack error")                \
  VISIT(SIGFPE,  FPE_INTDIV,  "integer divide-by-zero")              \
  VISIT(SIGFPE,  FPE_INTOVF,  "integer overflow")                    \
  VISIT(SIGFPE,  FPE_FLTDIV,  "floating point divide-by-zero")       \
  VISIT(SIGFPE,  FPE_FLTOVF,  "floating point overflow")             \
  VISIT(SIGFPE,  FPE_FLTUND,  "floating point underflow")            \
  VISIT(SIGFPE,  FPE_FLTRES,  "floating point inexact result")       \
  VISIT(SIGFPE,  FPE_FLTINV,  "invalid floating point operation")    \
  VISIT(SIGFPE,  FPE_FLTSUB,  "subscript out of range.")             \
  VISIT(SIGSEGV, SEGV_MAPERR, "address not mapped")                  \
  VISIT(SIGSEGV, SEGV_ACCERR, "invalid permissions")                 \
  VISIT(SIGBUS,  BUS_ADRALN,  "invalid address alignment")           \
  VISIT(SIGBUS,  BUS_ADRERR,  "non-existent physical address")       \
  VISIT(SIGBUS,  BUS_OBJERR,  "object-specific hardware error")      \
  VISIT(SIGTRAP, TRAP_BRKPT,  "process breakpoint")                  \
  VISIT(SIGTRAP, TRAP_TRACE,  "process trace trap")

struct SignalInfo {
  string signal;
  string code;
  string description;
};

static bool get_signal_info(siginfo_t *siginfo, SignalInfo &info) {
#define MAKE_CASE(s, c, d) if (siginfo->si_signo == s && siginfo->si_code == c) { \
  info.signal = #s; \
  info.code = #c; \
  info.description = d; \
  return true; \
}
eSignalCodes(MAKE_CASE);
#undef MAKE_CASE
return false;
}

/**
 * Removes all occurrences of the given substring from the string.
 */
static void remove_substring(char *str, string substr) {
  word p = 0;
  word i = 0;
  while (str[p]) {
    word j;
    for (j = 0; j < substr.length() && str[p + j] == substr[j]; j++)
      ;
    if (j < substr.length()) {
      str[i] = str[p];
      p++;
      i++;
    } else {
      p += j;
    }
  }
  str[i] = '\0';
}

static void print_stack_trace(void *ptr, string prefix) {
  ::fprintf(stderr, "--- Stack ---\n");
  void *pc = reinterpret_cast<void*>(__builtin_return_address(0));
  void **fp = reinterpret_cast<void**>(__builtin_frame_address(1));
  while (fp && pc) {
    Dl_info info;
    if (::dladdr(pc, &info)) {
      const char *mangled = info.dli_sname;
      int status = 0;
      own_ptr<char> demangled(abi::__cxa_demangle(mangled, 0, 0, &status));
      if (status == 0 && demangled.is_set()) {
        remove_substring(*demangled, prefix);
        ::fprintf(stderr, "%s\n", *demangled);
      } else {
        ::fprintf(stderr, "%s(...)\n", mangled);
        if (string("main") == mangled)
          break;
      }
    } else {
      ::fprintf(stderr, "<anonymous frame>\n");
    }
    pc = fp[1];
    fp = static_cast<void**>(fp[0]);
  }
}

static bool is_crash(word signo) {
  switch (signo) {
    case SIGSEGV: case SIGFPE: case SIGBUS:
      return true;
    default:
      return false;
  }
}

static void print_crash_report(int signum, siginfo_t *siginfo, void *ptr) {
  fprintf(stderr, "--- Crash ---\n");
  SignalInfo info;
  if (get_signal_info(siginfo, info)) {
    fprintf(stderr, "signal: %s\n", info.signal.start());
    fprintf(stderr, "code:   %s (%s)\n", info.code.start(), info.description.start());
  }
  switch (siginfo->si_signo) {
    case SIGSEGV: case SIGILL: case SIGFPE: case SIGBUS:
      fprintf(stderr, "addr:   %p\n", siginfo->si_addr);
      break;
  }
  if (siginfo->si_errno) {
    fprintf(stderr, "errno:  %s\n", strerror(errno));
  }
}

static void print_error_report(int signum, siginfo_t *siginfo, void *ptr) {
  // First uninstall signal handlers to avoid infinite regress if we
  // happen to crash during signal processing, and to allow the signal
  // sent using ::kill to be processed properly.
  Abort::uninstall_signal_handlers();
  if (is_crash(siginfo->si_signo))
    print_crash_report(signum, siginfo, ptr);
  print_stack_trace(ptr, "neutrino::");
  Abort::release_resources();
  // If we just ::exit here the process that spawned this one will
  // not be able to tell that we were stopped by a signal.  By killing
  // ourselves and letting the signal propagate, rather than handling
  // it, the parent process will be able to deal correctly with our
  // death.  Boy, that sounds morbid!  But it's all in this article:
  // http://www.cons.org/cracauer/sigint.html.
  ::kill(::getpid(), signum);
}

void Abort::uninstall_signal_handlers() {
  ::signal(SIGINT, SIG_DFL);
  ::signal(SIGFPE, SIG_DFL);
  ::signal(SIGBUS, SIG_DFL);
  ::signal(SIGABRT, SIG_DFL);
}

void Abort::install_signal_handlers() {
  bool success = true;
  // Dump information on crashes
  success = success && install_handler(SIGSEGV, print_error_report);
  success = success && install_handler(SIGFPE, print_error_report);
  success = success && install_handler(SIGBUS, print_error_report);
  success = success && install_handler(SIGABRT, print_error_report);
}

} // namespace neutrino