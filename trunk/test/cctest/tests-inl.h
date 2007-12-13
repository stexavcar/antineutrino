#ifndef _TEST_CCTEST_TESTS_INL
#define _TEST_CCTEST_TESTS_INL

#include "runtime/runtime.h"
#include "utils/nonlocal.h"

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
  LocalRuntime() { initialize(); }
};

/**
 * Checks that the vm aborts when performing the specified operation.
 * This is some nasty macro-magic but it is necessary because it uses
 * longjmp which is pretty picky about how you use it.
 */
#define CHECK_ABORTS(TYPE, operation)                                \
  do {                                                               \
    bool __has_aborted__ = false;                                    \
    bool __wrong_type__ = false;                                     \
    {                                                                \
      ConditionCatcher __catcher__;                                  \
      TRY (__catcher__) {                                            \
        (void) (operation);                                          \
      } CATCH (                                                      \
          case TYPE:                                                 \
            __has_aborted__ = true;                                  \
            break;                                                   \
          default:                                                   \
            __wrong_type__ = true;                                   \
            break;                                                   \
      );                                                             \
    }                                                                \
    CHECK(!__wrong_type__);                                          \
    CHECK(__has_aborted__);                                          \
  } while (false)

#define ASSERT_ABORTS(TYPE, operation) IF_DEBUG(CHECK_ABORTS(TYPE, operation))

} // namespace neutrino

#endif // _TEST_CCTEST_TESTS_INL
