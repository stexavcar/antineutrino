#ifndef _UTILS_POLLOCK
#define _UTILS_POLLOCK

// Pollock macros
#define assert


#ifdef DEBUG

#define pPositronAssertEq(left, right) cCheckEq(left, right)
#define pPositronAssertLt(left, right) cCheckLt(left, right)
#define pPositronAssertLeq(left, right) cCheckLeq(left, right)

#else

#define pPositronAssertEq(left, right)
#define pPositronAssertLt(left, right)
#define pPositronAssertLeq(left, right)

#endif

#endif // _UTILS_POLLOCK
