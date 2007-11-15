#ifndef _SPACE
#define _SPACE

#include "utils/types.h"

namespace neutrino {

class Space {
public:
  address allocate_raw(uint32_t size);
};

}

#endif // _SPACE
