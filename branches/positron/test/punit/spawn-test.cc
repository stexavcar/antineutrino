#include "platform/spawn.h"
#include "test-inl.h"
#include "plankton/builder-inl.h"
#include "plankton/plankton-inl.h"


#include <stdlib.h>


using namespace positron;


class TestChildProcess : public ChildProcess {
public:
  bool open(const string &test_name);
  ~TestChildProcess();
};


TestChildProcess::~TestChildProcess() {
  assert wait() == 0;
}


bool TestChildProcess::open(const string &test_name) {
  string self = UnitTest::args()[0];
  embed_vector<string, 3> args;
  args[0] = self;
  args[1] = "--spawned";
  string_stream full_test_name;
  full_test_name.add("spawn-test/%", positron::args(test_name));
  args[2] = full_test_name.raw_c_str();
  embed_vector<pair<string>, 2> env;
  env[0].set("foo", "bar");
  return ChildProcess::open(self, args, env);
}


TEST(hul_igennem) {
  TestChildProcess child;
  assert child.open("hul_igennem_child");
  for (word i = 0; i < 100; i++) {
    MessageBuffer buffer;
    p_value obj = buffer.receive(child.socket());
    assert is<p_integer>(obj);
    assert (cast<p_integer>(obj)).value() == i;
  }
  MessageBuffer buffer;
  p_value last = buffer.receive(child.socket());
  assert is<p_string>(last);
  assert (cast<p_string>(last)) == "bye!";
}


TEST(hul_igennem_child) {
  if (!UnitTest::spawned()) return;
  assert string("bar") == string(getenv("foo"));
  ParentProcess parent;
  assert parent.open();
  for (word i = 0; i < 100; i++) {
    MessageBuffer builder;
    builder.send(builder.new_integer(i), parent.socket());
  }
  MessageBuffer builder;
  builder.send(builder.new_string("bye!"), parent.socket());
}


TEST(waiting) {
  TestChildProcess child;
  assert child.open("waiting_child");
  {
    MessageBuffer builder;
    p_value message = builder.receive(child.socket());
    assert (cast<p_string>(message)) == string("ping");
  }
  // Wait .1 second to give the child a chance to wait.
  {
    MessageBuffer builder;
    builder.send(builder.new_string("pong"), child.socket());
  }
}


TEST(waiting_child) {
  if (!UnitTest::spawned()) return;
  ParentProcess parent;
  assert parent.open();
  // Wait .1 second to give the parent a chance to wait.
  {
    MessageBuffer builder;
    builder.send(builder.new_string("ping"), parent.socket());
  }
  {
    MessageBuffer builder;
    p_value message = builder.receive(parent.socket());
    assert (cast<p_string>(message)) == string("pong");
  }
}
