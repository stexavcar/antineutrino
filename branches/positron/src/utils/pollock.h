#ifndef _UTILS_POLLOCK
#define _UTILS_POLLOCK

// Pollock macros
#define assert


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

#endif // _UTILS_POLLOCK
