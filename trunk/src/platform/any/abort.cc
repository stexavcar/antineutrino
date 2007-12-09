#include "platform/abort.h"

#include <stdlib.h>

namespace neutrino {

void PlatformAbort::abort() {
  ::abort();
}

} // neutrino
