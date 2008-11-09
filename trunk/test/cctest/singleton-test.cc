#include "cctest/nunit-inl.h"
#include "runtime/runtime.h"
#include "values/values-inl.pp.h"

using namespace neutrino;

TEST(singletons) {
  LocalRuntime runtime;
  Roots &roots = runtime.roots();

  @check is<Void>(roots.vhoid());
  @check is<Null>(roots.nuhll());
  @check is<True>(roots.thrue());
  @check is<False>(roots.fahlse());

  CHECK(!is<Null>(roots.vhoid()));
  CHECK(!is<True>(roots.vhoid()));
  CHECK(!is<False>(roots.vhoid()));

  CHECK(!is<Void>(roots.nuhll()));
  CHECK(!is<True>(roots.nuhll()));
  CHECK(!is<False>(roots.nuhll()));
  
  CHECK(!is<Void>(roots.thrue()));
  CHECK(!is<Null>(roots.thrue()));
  CHECK(!is<False>(roots.thrue()));
  
  CHECK(!is<Void>(roots.fahlse()));
  CHECK(!is<Null>(roots.fahlse()));
  CHECK(!is<True>(roots.fahlse()));
}
