#ifndef _CCTEST_NUNIT_INL
#define _CCTEST_NUNIT_INL

#include "runtime/runtime.h"
#include "utils/nonlocal.h"
#include "cctest/nunit.h"

namespace neutrino {

/**
 * This class can be used in tests to check that a give operation
 * causes an abort.  When used with a TRY/CATCH from nonlocal.h it
 * catches aborts.  Note that, since the assertion facility also uses
 * aborts, you can't assert no the results until the AbortCatcher has
 * been deleted.  Also, since aborting leaves the vm in an
 * inconsistent state, you cannot use the vm in any way after an abort
 * has been caught.
 */
class ConditionCatcher : public Conditions, public NonLocal {
public:
  ConditionCatcher() { Conditions::set(this); }
  ~ConditionCatcher() { Conditions::set(NULL); }
  virtual void notify(Condition cause) { do_throw(cause); }
};

class LocalRuntime : public Runtime {
public:
  LocalRuntime() { initialize(NULL); }
};

/**
 * Checks that the vm aborts when performing the specified operation.
 * This is some nasty macro-magic but it is necessary because it uses
 * longjmp which is pretty picky about how you use it.
 */
#define CHECK_ABORTS(TYPE, operation)                                \
  do {                                                               \
    bool __has_aborted__ = false;                                    \
    uword __type__;                                                  \
    {                                                                \
      ConditionCatcher __catcher__;                                  \
      TRY (__catcher__) {                                            \
        (void) (operation);                                          \
      } CATCH (                                                      \
          __has_aborted__ = true;                                    \
          __type__ = TRY_CATCH_EXCEPTION();                          \
      );                                                             \
    }                                                                \
    CHECK(__has_aborted__);                                          \
    CHECK_EQ(TYPE, __type__);                                        \
  } while (false)

#define ASSERT_ABORTS(TYPE, operation) IF_DEBUG(CHECK_ABORTS(TYPE, operation))

#define TEST(name)                                                   \
  static void nunit_test_##name();                                   \
  static NUnitTest name(__FILE__, #name, nunit_test_##name);         \
  static void nunit_test_##name()

} // namespace neutrino

#endif // _CCTEST_NUNIT_INL
