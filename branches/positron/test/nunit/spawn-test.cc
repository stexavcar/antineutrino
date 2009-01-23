#include "platform/spawn.h"
#include "test-inl.h"
#include "plankton/builder-inl.h"
#include "plankton/plankton-inl.h"


#include <stdlib.h>


using namespace neutrino;


class TestChildProcess : public ChildProcess {
public:
  boole open(const string &test_name);
  ~TestChildProcess();
};


TestChildProcess::~TestChildProcess() {
  assert wait() == 0;
}


boole TestChildProcess::open(const string &test_name) {
  string self = UnitTest::args()[0];
  embed_vector<string, 3> args;
  args[0] = self;
  args[1] = "--spawned";
  string_stream full_test_name;
  full_test_name.add("spawn-test/%", neutrino::args(test_name));
  args[2] = full_test_name.raw_c_str();
  embed_vector<pair<string>, 2> env;
  env[0].set("foo", "bar");
  return ChildProcess::open(self, args, env);
}


TEST(hul_igennem) {
  TestChildProcess child;
  try child.open("hul_igennem_child");
  p_object proxy = child.proxy();
  for (word i = 0; i < 100; i++) {
    MessageOut buffer;
    p_value obj = proxy.send_sync(buffer.new_string("next_int"));
    assert is<p_integer>(obj);
    assert (cast<p_integer>(obj)).value() == i;
  }
  MessageOut buffer;
  p_value obj = proxy.send_sync(buffer.new_string("exit"));
  assert is<p_string>(obj);
  assert (cast<p_string>(obj)) == string("bye");
}


TEST(hul_igennem_child) {
  if (!UnitTest::spawned()) return;
  assert string("bar") == string(getenv("foo"));
  ParentProcess parent;
  try parent.open();
  word count = 0;
  while (true) {
    MessageIn message;
    try parent.receive(message);
    if (message.selector() == "next_int") {
      message.reply(MessageOut::new_integer(count++));
    } else if (message.selector() == "exit") {
      MessageOut buf;
      message.reply(buf.new_string("bye"));
      return;
    }
  }
}


TEST(sequence) {
  TestChildProcess child;
  try child.open("sequence_child");
  for (word i = 0; i < 1000; i++) {
    MessageOut message;
    p_value v = child.proxy().send_sync(message.new_string("ping"));
    assert (cast<p_integer>(v).value()) == i * 3;
  }
  for (word i = 0; i < 1000; i++) {
    ((int*) 0)[0] = 5;
    MessageIn message;
    child.receive(message);
    assert message.selector() == string("pong");
    message.reply(MessageOut::new_integer(i + 8));
  }
}


TEST(sequence_child) {
  if (!UnitTest::spawned()) return;
  ParentProcess parent;
  try parent.open();
  for (word i = 0; i < 1000; i++) {
    MessageIn message;
    try parent.receive(message);
    assert message.selector() == string("ping");
    message.reply(MessageOut::new_integer(i * 3));
  }
  for (word i = 0; i < 1000; i++) {
    MessageOut message;
    p_value v = parent.proxy().send_sync(message.new_string("pong"));
    assert (cast<p_integer>(v).value()) == i + 8;
  }
}


TEST(sync_auto_reply) {
  TestChildProcess child;
  try child.open("sync_auto_reply_child");
  for (word i = 0; i < 100; i++) {
    MessageOut message;
    p_value v = child.proxy().send_sync(message.new_string("pang"));
    assert is<p_void>(v);
  }
}


TEST(sync_auto_reply_child) {
  if (!UnitTest::spawned()) return;
  ParentProcess parent;
  try parent.open();
  for (word i = 0; i < 100; i++) {
    MessageIn message;
    try parent.receive(message);
    assert message.selector() == string("pang");
  }
}
