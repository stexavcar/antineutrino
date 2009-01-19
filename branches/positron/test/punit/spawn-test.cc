#include "platform/spawn.h"
#include "test-inl.h"
#include "plankton/builder-inl.h"
#include "plankton/plankton-inl.h"


#include <stdlib.h>


using namespace positron;


TEST(simple) {
  string self = UnitTest::args()[0];
  embed_vector<string, 3> args;
  args[0] = self;
  args[1] = "--spawned";
  args[2] = "spawn-test/child";
  embed_vector<pair<string>, 2> env;
  env[0].set("foo", "bar");
  ChildProcess child(self, args, env);
  assert child.open();
  for (word i = 0; i < 100; i++) {
    MessageBuffer buffer;
    p_value obj = buffer.receive(child.socket());
    assert is<p_integer>(obj);
    assert (cast<p_integer>(obj)).value() == i;
  }
  assert child.wait() == 0;
}


TEST(child) {
  if (!UnitTest::spawned()) return;
  assert string("bar") == string(getenv("foo"));
  ParentProcess parent;
  assert parent.open();
  for (word i = 0; i < 100; i++) {
    MessageBuffer builder;
    builder.send(builder.new_integer(i), parent.socket());
  }
}
