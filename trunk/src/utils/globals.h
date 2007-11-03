#ifndef _GLOBALS
#define _GLOBALS

#include <stdlib.h>
#include <stdio.h>

#include "utils/types.h"

// These macros should only ever be used on literal integers or
// fully parenthesized expressions.
#define KB * 1024
#define MB KB KB

#ifdef DEBUG
#define IF_DEBUG(action) action
#else
#define IF_DEBUG(action)
#endif

static const int kPointerSize = sizeof(void*);
static const int kWordSize = sizeof(word);

/**
 * Returns the new extended capacity given that the current capacity
 * is the given value.  The result approximates (to within 1%) an
 * increase in size by the golden ratio but is computed purely with
 * simple integer operations.
 */
static inline uint32_t grow_value(uint32_t n) {
  return ((n << 3) + (n << 2) + n) >> 3;
}


#endif _GLOBALS
