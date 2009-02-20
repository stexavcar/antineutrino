#ifndef _CCTEST_NUNIT_INL
#define _CCTEST_NUNIT_INL

#include "test.h"

namespace neutrino {

#define TEST(name)                                                   \
  static void punit_test_##name();                                   \
  static UnitTest punit_##name(__FILE__, #name, punit_test_##name);  \
  static void punit_test_##name()

#define TRY_TEST(name)                                               \
  static boole punit_test_##name();                                  \
  static void punit_bridge_##name() {                                \
    assert punit_test_##name().has_succeeded();                      \
  }                                                                  \
  static UnitTest punit_##name(__FILE__, #name, punit_bridge_##name);\
  static boole punit_test_##name()

} // namespace neutrino

#endif // _CCTEST_NUNIT_INL
