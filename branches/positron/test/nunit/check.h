#ifndef _PUNIT_UTILS_CHECK
#define _PUNIT_UTILS_CHECK

#include "utils/check-inl.h"

#define pNunitAssertEq(a, b) cCheckEq(a, b)
#define pNunitAssertNeq(a, b) cCheckNeq(a, b)
#define pNunitAssertLt(a, b) cCheckLt(a, b)
#define pNunitAssertLeq(a, b) cCheckLeq(a, b)
#define pNunitAssertGt(a, b) cCheckGt(a, b)
#define pNunitAssertGeq(a, b) cCheckGeq(a, b)
#define pNunitAssert(a) cCheck(a)
#define pNunitTry(a) cCheck(a.has_succeeded())

#endif // _PUNIT_UTILS_CHECK
