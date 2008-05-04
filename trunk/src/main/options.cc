#include "main/options.h"
#include "utils/list-inl.h"

namespace neutrino {

DEFINE_FLAG(bool, Options, print_stats_on_exit, false);

COND_DEFINE_FLAG(IS_PARANOID, bool, Options, check_stack_height, false);

COND_DEFINE_FLAG(IS_DEBUG, bool, Options, trace_interpreter, false);

DEFINE_FLAG(list<string>, Options, images, list<string>());

DEFINE_FLAG(list<string>, Options, libs, list<string>());

DEFINE_FLAG(list<string>, Options, args, list<string>());

} // neutrino
