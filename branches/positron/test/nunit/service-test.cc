#include "io/miniheap-inl.h"
#include "plankton/plankton-inl.h"
#include "test-inl.h"

using namespace neutrino;

/*

class TestObjectProxy : public ObjectProxyDTable<TestObjectProxy> {
public:
  ObjectProxyDTable<TestObjectProxy> &dtable() { return *this; }
};

static p::Object make_test_service() {
  static TestObjectProxy *proxy = NULL;
  if (proxy == NULL)
    proxy = new TestObjectProxy();
  return proxy->dtable().to_object(*proxy);
}

REGISTER_SERVICE(nunit.test, make_test_service);

TEST(service_lookup) {
  p::Object obj = p::ServiceRegistry::lookup("nunit.test");
  assert (is<p::Null>(obj.send_sync("foo")));
}

*/
