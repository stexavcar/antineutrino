#ifndef _PLATFORM_POSIX_ABORT
#define _PLATFORM_POSIX_ABORT

#include "platform/abort.h"
#include "utils/checks.h"
#include "utils/types.h"

#if defined(M64) && defined(__STRICT_ANSI__)
// On 64-bit linux with strict ansi __*64 do not get defined for some
// reason.
typedef word __s64;
typedef uword __u64;
STATIC_CHECK(sizeof(__s64) == 8);
STATIC_CHECK(sizeof(__u64) == 8);
#endif

#include <signal.h>
#include <stdlib.h>
#include <string.h>

namespace neutrino {

#define FOR_EACH_SIGNAL(VISIT)                                       \
  VISIT(SIGHUP,    "terminal line hangup")                           \
  VISIT(SIGINT,    "interrupt program")                              \
  VISIT(SIGQUIT,   "quit program")                                   \
  VISIT(SIGILL,    "illegal instruction")                            \
  VISIT(SIGTRAP,   "trace trap")                                     \
  VISIT(SIGABRT,   "abort program (formerly SIGIOT)")                \
  VISIT(SIGFPE,    "floating-point exception")                       \
  VISIT(SIGKILL,   "kill program")                                   \
  VISIT(SIGBUS,    "bus error")                                      \
  VISIT(SIGSEGV,   "segmentation violation")                         \
  VISIT(SIGSYS,    "non-existent system call invoked")               \
  VISIT(SIGPIPE,   "write on a pipe with no reader")                 \
  VISIT(SIGALRM,   "real-time timer expired")                        \
  VISIT(SIGTERM,   "software termination signal")                    \
  VISIT(SIGURG,    "urgent condition present on socket")             \
  VISIT(SIGSTOP,   "stop (cannot be caught or ignored)")             \
  VISIT(SIGTSTP,   "stop signal generated from keyboard")            \
  VISIT(SIGCONT,   "continue after stop")                            \
  VISIT(SIGCHLD,   "child status has changed")                       \
  VISIT(SIGTTIN,   "background read attempted from control terminal")\
  VISIT(SIGTTOU,   "background write attempted to control terminal") \
  VISIT(SIGIO,     "I/O is possible on a descriptor (see fcntl(2))") \
  VISIT(SIGXCPU,   "cpu time limit exceeded (see setrlimit(2))")     \
  VISIT(SIGXFSZ,   "file size limit exceeded (see setrlimit(2))")    \
  VISIT(SIGVTALRM, "virtual time alarm (see setitimer(2))")          \
  VISIT(SIGPROF,   "profiling timer alarm (see setitimer(2))")       \
  VISIT(SIGWINCH,  "Window size change")                             \
  VISIT(SIGUSR1,   "User defined signal 1")                          \
  VISIT(SIGUSR2,   "User defined signal 2")

#define FOR_EACH_SIGNAL_CODE(VISIT)                                  \
  VISIT(0,       SI_USER,     "User signal")                         \
  VISIT(SIGILL,  ILL_ILLOPC,  "Illegal opcode")                      \
  VISIT(SIGILL,  ILL_ILLOPN,  "Illegal operand")                     \
  VISIT(SIGILL,  ILL_ILLADR,  "Illegal addressing mode")             \
  VISIT(SIGILL,  ILL_ILLTRP,  "Illegal trap")                        \
  VISIT(SIGILL,  ILL_PRVOPC,  "Privileged opcode")                   \
  VISIT(SIGILL,  ILL_PRVREG,  "Privileged register")                 \
  VISIT(SIGILL,  ILL_COPROC,  "Coprocessor error")                   \
  VISIT(SIGILL,  ILL_BADSTK,  "Internal stack error")                \
  VISIT(SIGFPE,  FPE_INTDIV,  "Integer divide-by-zero")              \
  VISIT(SIGFPE,  FPE_INTOVF,  "Integer overflow")                    \
  VISIT(SIGFPE,  FPE_FLTDIV,  "Floating point divide-by-zero")       \
  VISIT(SIGFPE,  FPE_FLTOVF,  "Floating point overflow")             \
  VISIT(SIGFPE,  FPE_FLTUND,  "Floating point underflow")            \
  VISIT(SIGFPE,  FPE_FLTRES,  "Floating point inexact result")       \
  VISIT(SIGFPE,  FPE_FLTINV,  "Invalid floating point operation")    \
  VISIT(SIGFPE,  FPE_FLTSUB,  "Subscript out of range.")             \
  VISIT(SIGSEGV, SEGV_MAPERR, "Address not mapped")                  \
  VISIT(SIGSEGV, SEGV_ACCERR, "Invalid permissions")                 \
  VISIT(SIGBUS,  BUS_ADRALN,  "Invalid address alignment")           \
  VISIT(SIGBUS,  BUS_ADRERR,  "Non-existent physical address")       \
  VISIT(SIGBUS,  BUS_OBJERR,  "Object-specific hardware error")      \
  VISIT(SIGTRAP, TRAP_BRKPT,  "Process breakpoint")                  \
  VISIT(SIGTRAP, TRAP_TRACE,  "Process trace trap")

} // neutrino

#endif // _PLATFORM_POSIX_ABORT
