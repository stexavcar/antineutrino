#include "io/miniheap-inl.h"
#include "plankton/plankton-inl.h"
#include "test-inl.h"

using namespace neutrino;

class TestService {
public:
  TestService();
  ObjectProxyDTable<TestService> &dtable() { return dtable_; }
  p::Value echo(Message &message);
private:
  ObjectProxyDTable<TestService> dtable_;
};

TestService::TestService() {
  dtable().add_method("echo", &TestService::echo);
}

p::Value TestService::echo(Message &message) {
  assert message.args().length() == 1;
  return message.args()[0];
}

static p::Object make_test_service() {
  static TestService instance;
  return instance.dtable().proxy_for(instance);
}

REGISTER_SERVICE(nunit.test.echo, make_test_service);

TEST(service_lookup) {
  p::Object obj = p::ServiceRegistry::lookup("nunit.test.echo");
  PseudoRandom random(76);
  for (word i = 0; i < 10; i++) {
    word n = random.next() % (1 << 24);
    p::Value response = obj.send("echo", p::Array::of(n));
    assert (is<p::Integer>(response));
    assert (cast<p::Integer>(response)).value() == n;
  }
  assert (is<p::Null>(obj.send("some_undefined_message")));
}
