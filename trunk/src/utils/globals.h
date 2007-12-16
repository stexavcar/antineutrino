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
#define IF_NOT_DEBUG(arg) typedef void SEMI_STATIC_JOIN(__IfNotDebug__, __LINE__)
#define IF_ELSE_DEBUG(t, e) t
#else
#define IF_DEBUG(arg) typedef void SEMI_STATIC_JOIN(__IfDebug__, __LINE__)
#define IF_NOT_DEBUG(arg) arg
#define IF_ELSE_DEBUG(t, e) e
#endif

#ifdef PARANOID
#define IF_PARANOID(arg) arg
#define IF_ELSE_PARANOID(t, e) t
#else
#define IF_PARANOID(arg) typedef void SEMI_STATIC_JOIN(__IfParanoid__, __LINE__)
#define IF_ELSE_PARANOID(t, e) e
#endif

template <typename T>
static inline void USE(T t) { }

static const uint32_t kPointerSize = sizeof(void*);
static const uint32_t kWordSize = sizeof(word);

static const bool kDebugMode = IF_ELSE_DEBUG(true, false);

/**
 * Returns the new extended capacity given that the current capacity
 * is the given value.  The result approximates (to within 1%) an
 * increase in size by the golden ratio but is computed purely with
 * simple integer operations.
 */
static inline uint32_t grow_value(uint32_t n) {
  return ((n << 3) + (n << 2) + n) >> 3;
}

template <typename T>
static inline T max(T a, T b) {
  return (a < b) ? b : a;
}

#endif // _GLOBALS
