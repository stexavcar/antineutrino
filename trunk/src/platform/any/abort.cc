#include "platform/abort.h"

#include <stdlib.h>

namespace neutrino {

void Abort::setup_signal_handler() {
  
}

void Abort::abort() {
  ::abort();
}

} // neutrino
