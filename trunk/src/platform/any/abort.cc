#include "platform/abort.h"

#include <stdlib.h>

namespace neutrino {

bool Abort::setup_signal_handler() {
  return true;
}

void Abort::abort() {
  ::abort();
}

} // neutrino
