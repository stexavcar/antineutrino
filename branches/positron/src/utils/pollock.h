#ifndef _UTILS_POLLOCK
#define _UTILS_POLLOCK

// Pollock macros
#define assert
#define try


#ifdef DEBUG

#define pPositronAssertEq(left, right) cCheckEq(left, right)
#define pPositronAssertNeq(left, right) cCheckNeq(left, right)
#define pPositronAssertLt(left, right) cCheckLt(left, right)
#define pPositronAssertLeq(left, right) cCheckLeq(left, right)
#define pPositronAssertGeq(left, right) cCheckGeq(left, right)
#define pPositronAssert(expr) cCheck(expr)

#else

#define pPositronAssertEq(left, right)
#define pPositronAssertNeq(left, right)
#define pPositronAssertLt(left, right)
#define pPositronAssertLeq(left, right)
#define pPositronAssertGeq(left, right)
#define pPositronAssert(expr)

#endif

#define pPositronTry(__expr__)                                       \
  do {                                                               \
    boole __value__ = (__expr__);                                    \
    if (!__value__.has_succeeded())                                  \
      return __value__;                                              \
  } while (false)

#endif // _UTILS_POLLOCK
