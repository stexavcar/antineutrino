#include "cctest/tests-inl.h"
#include "io/out-stream.h"
#include "runtime/runtime.h"
#include "values/channel.h"
#include "values/values-inl.h"
#include "utils/checks.h"
#include "utils/string-inl.h"

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


void Test::tuple() {
  LocalRuntime runtime;
  
  String *str1 = cast<String>(runtime.heap().new_string("one"));
  String *str2 = cast<String>(runtime.heap().new_string("two"));
  Tuple *tup = cast<Tuple>(runtime.heap().new_tuple(2));
  tup->set(0, str1);
  tup->set(1, str2);
  
  Serializer stream;
  RawFValue *raw_tup = stream.marshal(tup);
  stream.flush();
  use(raw_tup);
}


void Test::simple_objects() {
  LocalRuntime runtime;
  Smi *smi_obj = Smi::from_int(10);
  string knirk = "knirk";
  String *str_obj = cast<String>(runtime.heap().new_string(knirk));
  Tuple *emp_obj = cast<Tuple>(runtime.heap().new_tuple(0));

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
  CHECK_EQ(p::Value::vtInteger, smi_val.type());
  CHECK(p::is<p::Integer>(smi_val));
  CHECK(!p::is<p::String>(smi_val));
  CHECK(!p::is<p::Tuple>(smi_val));
  p::Integer smi = p::cast<p::Integer>(smi_val);
  CHECK_EQ(10, smi.value());
  
  p::Value str_val = ApiUtils::new_value(dict, f_str);
  CHECK_EQ(p::Value::vtString, str_val.type());
  CHECK(!p::is<p::Integer>(str_val));
  CHECK(p::is<p::String>(str_val));
  CHECK(!p::is<p::Tuple>(str_val));
  p::String str = p::cast<p::String>(str_val);
  CHECK_EQ(knirk.length(), str.length());
  const char *c_str = str.c_str();
  for (unsigned i = 0; i < str.length(); i++) {
    CHECK_EQ(knirk[i], str[i]);
    CHECK_EQ(knirk[i], c_str[i]);
  }
  CHECK_EQ(0, c_str[str.length()]);
  
  p::Value emp_val = ApiUtils::new_value(dict, f_emp);
  CHECK_EQ(p::Value::vtTuple, emp_val.type());
  CHECK(!p::is<p::Integer>(emp_val));
  CHECK(!p::is<p::String>(emp_val));
  CHECK(p::is<p::Tuple>(emp_val));
  p::Tuple emp = p::cast<p::Tuple>(emp_val);
  CHECK_EQ(0, emp.length());
}
