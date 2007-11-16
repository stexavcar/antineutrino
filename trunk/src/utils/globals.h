#ifndef _GLOBALS
#define _GLOBALS

#include <stdlib.h>
#include <stdio.h>

#include "utils/types.h"

// These macros should only ever be used on literal integers or
// fully parenthesized expressions.
#define KB * 1024
#define MB KB KB

// This macro joins two tokens.  If one of the tokens is a macro the
// helper call causes it to be resolved before joining.
#define SEMI_STATIC_JOIN(a, b) SEMI_STATIC_JOIN_HELPER(a, b)
#define SEMI_STATIC_JOIN_HELPER(a, b) a##b

#ifdef DEBUG
#define IF_DEBUG(arg) arg
#else
#define IF_DEBUG(arg) typedef void SEMI_STATIC_JOIN(__IfDebug__, __LINE__)
#endif

template <typename T>
static inline void USE(T t) { }

static const uint32_t kPointerSize = sizeof(void*);
static const uint32_t kWordSize = sizeof(word);

/**
 * Returns the new extended capacity given that the current capacity
 * is the given value.  The result approximates (to within 1%) an
 * increase in size by the golden ratio but is computed purely with
 * simple integer operations.
 */
static inline uint32_t grow_value(uint32_t n) {
  return ((n << 3) + (n << 2) + n) >> 3;
}


#endif // _GLOBALS
