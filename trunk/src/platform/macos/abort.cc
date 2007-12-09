#include "platform/abort.h"

#include <stdlib.h>

namespace neutrino {

void Abort::abort() {
  ::abort();
}

} // neutrino
