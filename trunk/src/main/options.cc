#include "main/options.h"
#include "utils/list-inl.h"

namespace neutrino {

bool Options::print_stats_on_exit = false;
REGISTER_FLAG(bool, Options, print_stats_on_exit);

list<string> Options::images;
REGISTER_FLAG(list<string>, Options, images);

} // neutrino
