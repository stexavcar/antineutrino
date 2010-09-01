#include "utils/nonlocal.h"

namespace neutrino {

NonLocal::NonLocal() { }

void NonLocal::do_throw(int value) {
  longjmp(jump_buffer(), value);
}

}
