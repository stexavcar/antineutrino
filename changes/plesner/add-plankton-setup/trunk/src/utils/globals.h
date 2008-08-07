#ifndef _UTILS_GLOBALS
#define _UTILS_GLOBALS

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

// If the first argument is true then evaluates to the second 
// argument, otherwise the third
#define IF(COND, then_part, else_part) COND(then_part, else_part)

#ifdef DEBUG
static const bool kDebug = true;
#define IF_DEBUG(arg)       arg
#define IF_NOT_DEBUG(arg)   typedef void SEMI_STATIC_JOIN(__IfNotDebug__, __LINE__)
#define IS_DEBUG(t, e) t
#else
static const bool kDebug = false;
#define IF_DEBUG(arg)       typedef void SEMI_STATIC_JOIN(__IfDebug__, __LINE__)
#define IF_NOT_DEBUG(arg)   arg
#define IS_DEBUG(t, e) e
#endif

#ifdef PARANOID
static const bool kParanoid = true;
#define IF_PARANOID(arg)     arg
#define IF_NOT_PARANOID(arg) typedef void SEMI_STATIC_JOIN(__IfNotParanoid__, __LINE__)
#define IS_PARANOID(t, e)    t
#else
static const bool kParanoid = false;
#define IF_PARANOID(arg)     typedef void SEMI_STATIC_JOIN(__IfParanoid__, __LINE__)
#define IF_NOT_PARANOID(arg) arg
#define IS_PARANOID(t, e)    e
#endif

template <typename T>
static inline void use(T t) { }

static const uword kPointerSize = sizeof(void*);
static const uword kWordSize = sizeof(word);
static const uword kBitsPerByte = 8;

static const bool kDebugMode = IF(IS_DEBUG, true, false);

/**
 * Returns the new extended capacity given that the current capacity
 * is the given value.  The result approximates (to within 1%) an
 * increase in size by the golden ratio but is computed purely with
 * simple integer operations.
 */
static inline uword grow_value(uword n) {
  return ((n << 3) + (n << 2) + n) >> 3;
}

template <typename T>
static inline T max(T a, T b) {
  return (a < b) ? b : a;
}

#endif // _UTILS_GLOBALS
