#include <cstdio>
#include <cstdlib>
#include <signal.h>
#include <errno.h>

#include "platform/abort.h"
#include "utils/string-inl.h"

namespace positron {

void Abort::abort(string format, const var_args &vars) {
  string_stream buf;
  buf.add(format, vars);
  fprintf(stderr, "%s", buf.raw_c_str().start());
  ::abort();
}

static bool install_handler(int signum, void (*handler)(int, siginfo_t*, void*)) {
  struct sigaction action;
  memset(&action, 0, sizeof(action));
  action.sa_sigaction = handler;
  action.sa_flags = SA_SIGINFO;
  return sigaction(signum, &action, NULL) >= 0;
}

/*
fprintf(stderr, "--- Crash ---\n");
EnumValueInfo enum_info;
if (get_signal_info(info->si_signo, &enum_info))
  fprintf(stderr, "signal: %s (%s)\n", enum_info.name, enum_info.desc);
if (get_code_info(info->si_signo, info->si_code, &enum_info))
  fprintf(stderr, "code:   %s (%s)\n", enum_info.name, enum_info.desc);
switch (info->si_signo) {
  case SIGSEGV: case SIGILL: case SIGFPE: case SIGBUS:
    fprintf(stderr, "addr:   %x\n", static_cast<uint32_t>(reinterpret_cast<uword>(info->si_addr)));
    break;
}
fprintf(stderr, "status: %i\n", info->si_status);
if (info->si_errno)
  fprintf(stderr, "errno:  %s\n", strerror(errno));
print_stack_trace(ptr, "neutrino::");
exit(signum);
*/

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

static void print_error_report(int signum, siginfo_t *siginfo, void *ptr) {
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
  exit(signum);
}

void Abort::install_signal_handlers() {
  bool success = true;
  // Dump information on crashes
  success = success && install_handler(SIGSEGV, print_error_report);
  success = success && install_handler(SIGFPE, print_error_report);
  success = success && install_handler(SIGBUS, print_error_report);
}

} // namespace positron
