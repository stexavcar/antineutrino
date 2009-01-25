#include "io/miniheap-inl.h"
#include "plankton/plankton-inl.h"
#include "test-inl.h"

using namespace neutrino;

class TestServiceDTable : public p::Value::DTable {
public:
  TestServiceDTable();
  static p::Value send(p::Object obj, p::String name,
      p::Array args, bool is_synchronous);
};

TestServiceDTable::TestServiceDTable() {
  object.send = send;
}

p::Value TestServiceDTable::send(p::Object obj, p::String name,
    p::Array args, bool is_sync) {
  return MessageOut::get_null();
}

static p::Object make_test_service() {
  static TestServiceDTable *dtable = new TestServiceDTable();
  return p::Object(0, dtable);
}

REGISTER_SERVICE(nunit.test, make_test_service);

TEST(service_lookup) {
  p::Object obj = p::ServiceRegistry::lookup("nunit.test");
  assert (is<p::Null>(obj.send_sync("foo")));
}
