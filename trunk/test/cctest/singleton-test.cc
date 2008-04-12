#include "cctest/tests-inl.h"
#include "runtime/runtime.h"
#include "values/values-inl.h"

using namespace neutrino;

void Test::singletons() {
  LocalRuntime runtime;
  Roots &roots = runtime.roots();

  CHECK_IS(Void, roots.vhoid());
  CHECK_IS(Null, roots.nuhll());
  CHECK_IS(True, roots.thrue());
  CHECK_IS(False, roots.fahlse());

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
