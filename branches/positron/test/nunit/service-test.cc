#include "plankton/builder-inl.h"
#include "plankton/plankton-inl.h"
#include "test-inl.h"

using namespace neutrino;

class TestServiceDTable : public p_value::DTable {
public:
  TestServiceDTable();
  static p_value send(const p_object *obj, p_string name, p_array args,
      bool is_synchronous);
};

TestServiceDTable::TestServiceDTable() {
  object.send = send;
}

p_value TestServiceDTable::send(const p_object *obj, p_string name,
    p_array args, bool is_sync) {
  return MessageOut::get_null();
}

static p_object make_test_service() {
  static TestServiceDTable *dtable = new TestServiceDTable();
  return p_object(0, dtable);
}

REGISTER_SERVICE(nunit.test, make_test_service);

TEST(service_lookup) {
  p_object obj = ServiceRegistry::lookup("nunit.test");
  MessageOut out;
  assert (is<p_null>(obj.send_sync(out.new_string("foo"))));
}
