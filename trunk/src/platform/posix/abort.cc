#include "platform/posix/abort.h"
#include "utils/conditions.h"
#include "utils/globals.h"

#include <errno.h>

namespace neutrino {

static void print_stack_trace(void *addr, const char *prefix);

struct EnumValueInfo {
  char *name;
  char *desc;
};

static bool get_signal_info(int signum, EnumValueInfo *info) {
  switch (signum) {
#define MAKE_CASE(SIG, str) case SIG: { info->name = #SIG; info->desc = str; return true; }
FOR_EACH_SIGNAL(MAKE_CASE)
#undef MAKE_CASE
default:
    return false;
  }
}

static bool get_code_info(int signum, int code, EnumValueInfo *info) {
#define MAKE_CASE(SIG, CODE, str)                                    \
  if ((SIG == 0 || (signum == SIG)) && (code == CODE)) {             \
    info->name = #CODE;                                              \
    info->desc = str;                                                \
    return true;                                                     \
  }
FOR_EACH_SIGNAL_CODE(MAKE_CASE)
#undef MAKE_CASE
  return false;
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

static void print_error_report(int signum, siginfo_t *info, void *ptr) {
  fprintf(stderr, "--- Crash ---\n");
  EnumValueInfo enum_info;
  if (get_signal_info(info->si_signo, &enum_info))
    fprintf(stderr, "signal: %s (%s)\n", enum_info.name, enum_info.desc);
  if (get_code_info(info->si_signo, info->si_code, &enum_info))
    fprintf(stderr, "code:   %s (%s)\n", enum_info.name, enum_info.desc);    
  switch (info->si_signo) {
    case SIGSEGV: case SIGILL: case SIGFPE: case SIGBUS:
      fprintf(stderr, "addr:   %x\n", reinterpret_cast<uint32_t>(info->si_addr));
      break;
  }
  fprintf(stderr, "status: %i\n", info->si_status);
  if (info->si_errno)
    fprintf(stderr, "errno:  %s\n", strerror(errno));
  print_stack_trace(ptr, "neutrino::");
  exit(signum);
}

static bool install_handler(int signum, void (*handler)(int, siginfo_t*, void*)) {
  struct sigaction action;
  memset(&action, 0, sizeof(action));
  action.sa_sigaction = handler;
  action.sa_flags = SA_SIGINFO;  
  return sigaction(signum, &action, NULL) >= 0;
}

bool Abort::setup_signal_handler() {
  bool success = true;
  success = success && install_handler(SIGSEGV, print_error_report);
  success = success && install_handler(SIGFPE, print_error_report);
  if (!success) {
    Conditions::get().error_occurred("Error setting up signal handlers.");
    return false;
  }
  return true;
}

} // neutrino
