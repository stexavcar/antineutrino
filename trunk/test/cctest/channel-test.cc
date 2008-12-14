#include "cctest/nunit-inl.h"
#include "io/out-stream.h"
#include "runtime/runtime.h"
#include "values/channel.h"
#include "values/values-inl.pp.h"
#include "utils/checks.h"
#include "utils/string-inl.pp.h"

namespace p = plankton;
using namespace neutrino;

// This test is not for running, only compiling
void walk_dont_run() {
  p::Value value = *static_cast<p::Value*>(NULL);
  p::is<p::String>(value);
  p::is<p::Tuple>(value);
  p::cast<p::String>(value);
  p::cast<p::Tuple>(value);
}


TEST(tuple) {
  LocalRuntime runtime;

  String *str1 = runtime.heap().new_string("one").value();
  String *str2 = runtime.heap().new_string("two").value();
  Tuple *tup = runtime.heap().new_tuple(2).value();
  tup->set(0, str1);
  tup->set(1, str2);

  Serializer stream;
  RawFValue *raw_tup = stream.marshal(tup);
  stream.flush();
  use(raw_tup);
}


TEST(simple_objects) {
  LocalRuntime runtime;
  Smi *smi_obj = Smi::from_int(10);
  string knirk = "knirk";
  String *str_obj = runtime.heap().new_string(knirk).value();
  Tuple *emp_obj = runtime.heap().new_tuple(0).value();

  Serializer stream;
  RawFValue *raw_smi = stream.marshal(smi_obj);
  RawFValue *raw_str = stream.marshal(str_obj);
  RawFValue *raw_emp = stream.marshal(emp_obj);
  stream.flush();
  FrozenHeap heap(stream);
  FImmediate *f_smi = heap.cook(raw_smi);
  FImmediate *f_str = heap.cook(raw_str);
  FImmediate *f_emp = heap.cook(raw_emp);
  ExtendedValueDTable &dict = FrozenValueDTableImpl::instance();

  p::Value smi_val = ApiUtils::new_value(dict, f_smi);
  @check smi_val.type() == p::Value::vtInteger;
  CHECK(p::is<p::Integer>(smi_val));
  CHECK(!p::is<p::String>(smi_val));
  CHECK(!p::is<p::Tuple>(smi_val));
  p::Integer smi = p::cast<p::Integer>(smi_val);
  @check smi.value() == 10;

  p::Value str_val = ApiUtils::new_value(dict, f_str);
  @check str_val.type() == p::Value::vtString;
  CHECK(!p::is<p::Integer>(str_val));
  CHECK(p::is<p::String>(str_val));
  CHECK(!p::is<p::Tuple>(str_val));
  p::String str = p::cast<p::String>(str_val);
  @check str.length() == knirk.length();
  const char *c_str = str.c_str();
  for (unsigned i = 0; i < str.length(); i++) {
    @check str[i] == knirk[i];
    @check c_str[i] == knirk[i];
  }
  @check c_str[str.length()] == 0;

  p::Value emp_val = ApiUtils::new_value(dict, f_emp);
  @check emp_val.type() == p::Value::vtTuple;
  CHECK(!p::is<p::Integer>(emp_val));
  CHECK(!p::is<p::String>(emp_val));
  CHECK(p::is<p::Tuple>(emp_val));
  p::Tuple emp = p::cast<p::Tuple>(emp_val);
  @check emp.length() == 0;
}
