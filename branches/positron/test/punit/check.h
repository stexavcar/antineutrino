#ifndef _PUNIT_UTILS_CHECK
#define _PUNIT_UTILS_CHECK

#define pPunitAssertEq(a, b) cCheckEq(a, b)
#define pPunitAssertNeq(a, b) cCheckNeq(a, b)
#define pPunitAssertLt(a, b) cCheckLt(a, b)
#define pPunitAssertLeq(a, b) cCheckLeq(a, b)
#define pPunitAssertGt(a, b) cCheckGt(a, b)
#define pPunitAssertGeq(a, b) cCheckGeq(a, b)
#define pPunitAssert(a) cCheck(a)

#endif // _PUNIT_UTILS_CHECK
