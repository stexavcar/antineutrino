#include "cctest/nunit-inl.h"
#include "runtime/runtime.h"
#include "values/values-inl.pp.h"

using namespace neutrino;

TEST(runtime_creation) {
  Runtime runtime;
  CHECK(!runtime.initialize(NULL).has_failed());
  @check is<Layout>(runtime.roots().layout_layout());
  @check is<Layout>(runtime.roots().string_layout());
}
