#include "platform/posix/abort.h"
#include "utils/globals.h"
#include "utils/log.h"

#include <errno.h>

namespace neutrino {

static void print_stack_trace(void *addr, const char *prefix);

Resource *Abort::first_ = NULL;
Resource *Abort::last_ = NULL;

struct EnumValueInfo {
  const char *name;
  const char *desc;
};

static bool get_signal_info(int signum, EnumValueInfo *info) {
  switch (signum) {
#define MAKE_CASE(SIG, str) case SIG: { info->name = #SIG; info->desc = str; return true; }
eSignals(MAKE_CASE)
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
eSignalCodes(MAKE_CASE)
#undef MAKE_CASE
  return false;
}

/**
 * Removes all occurrences of the given substring from the string.
 */
static void remove_substring(char *str, const char *substr) {
  uword p = 0;
  uword i = 0;
  while (str[p]) {
    uword j;
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

static bool cleaned_up = false;
void Abort::cleanup_resources() {
  if (cleaned_up) return;
  cleaned_up = true;
  Resource *current = Abort::first_;
  while (current != NULL) {
    current->cleanup();
    current = current->next_;
  }
}

void Resource::install() {
  prev_ = Abort::last_;
  next_ = NULL;
  if (Abort::first_ == NULL) Abort::first_ = this;
  if (Abort::last_ != NULL) Abort::last_->next_ = this;
  Abort::last_ = this;
}

void Resource::uninstall() {
  if (prev_ == NULL) Abort::first_ = next_;
  else prev_->next_ = next_;
  if (next_ == NULL) Abort::last_ = prev_;
  else next_->prev_ = prev_;
}

static void signal_cleanup_resources(int signum, siginfo_t *info, void *ptr) {
  Abort::cleanup_resources();
  ::exit(signum);
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
      fprintf(stderr, "addr:   %x\n", static_cast<uint32_t>(reinterpret_cast<uword>(info->si_addr)));
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
  // Dump information on crashes
  success = success && install_handler(SIGSEGV, print_error_report);
  success = success && install_handler(SIGFPE, print_error_report);
  success = success && install_handler(SIGBUS, print_error_report);
  // Make sure to clean up nicely on interruption
  success = success && install_handler(SIGINT, signal_cleanup_resources);
  success = success && install_handler(SIGTERM, signal_cleanup_resources);
  atexit(cleanup_resources);
  if (!success) {
    LOG().error("Error setting up signal handlers.", elms());
    return false;
  }
  return true;
}

} // neutrino
