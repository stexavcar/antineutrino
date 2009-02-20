#ifndef _UTILS_POLLOCK
#define _UTILS_POLLOCK

// Pollock macros
#if !defined(M32) && !defined(M64)
#define assert
#define try
#define parse
#define safealloc
#define alloc
#define in ;
#endif

#ifdef DEBUG

#define pNeutrinoAssertEq(left, right) cCheckEq(left, right)
#define pNeutrinoAssertNeq(left, right) cCheckNeq(left, right)
#define pNeutrinoAssertLt(left, right) cCheckLt(left, right)
#define pNeutrinoAssertLeq(left, right) cCheckLeq(left, right)
#define pNeutrinoAssertGt(left, right) cCheckGt(left, right)
#define pNeutrinoAssertGeq(left, right) cCheckGeq(left, right)
#define pNeutrinoAssert(expr) cCheck(expr)

#else

#define pNeutrinoAssertEq(left, right)
#define pNeutrinoAssertNeq(left, right)
#define pNeutrinoAssertLt(left, right)
#define pNeutrinoAssertLeq(left, right)
#define pNeutrinoAssertGt(left, right)
#define pNeutrinoAssertGeq(left, right)
#define pNeutrinoAssert(expr)

#endif

#define pTryDeclare(__Type__, __name__, __value__)                   \
  __Type__ *__name__ = NULL;                                         \
  do {                                                               \
    allocation<__Type__> __result__ = __value__;                     \
    if (__result__.has_failed())                                     \
      return __result__.failure();                                   \
    __name__ = __result__.value();                                   \
  } while (false)

#define pNeutrinoTry(__expr__)                                       \
  do {                                                               \
    boole __value__ = (__expr__);                                    \
    if (!__value__.has_succeeded())                                  \
      return __value__.failure();                                    \
  } while (false)

#endif // _UTILS_POLLOCK
