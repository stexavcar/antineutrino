#include "cctest/tests-inl.h"
#include "io/out-stream.h"
#include "runtime/runtime.h"
#include "values/channel.h"
#include "values/values-inl.h"
#include "utils/checks.h"
#include "utils/string-inl.h"

using namespace neutrino;

// This test is not for running, only compiling
void walk_dont_run() {
  NValue value = *static_cast<NValue*>(NULL);
  is<NString>(value);
  is<NTuple>(value);
  cast<NString>(value);
  cast<NTuple>(value);
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
  USE(raw_tup);
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
  
  NValue smi_val = ApiUtils::new_value(dict, f_smi);
  CHECK_EQ(vtInteger, smi_val.type());
  CHECK(is<NInteger>(smi_val));
  CHECK(!is<NString>(smi_val));
  CHECK(!is<NTuple>(smi_val));
  NInteger smi = cast<NInteger>(smi_val);
  CHECK_EQ(10, smi.value());
  
  NValue str_val = ApiUtils::new_value(dict, f_str);
  CHECK_EQ(vtString, str_val.type());
  CHECK(!is<NInteger>(str_val));
  CHECK(is<NString>(str_val));
  CHECK(!is<NTuple>(str_val));
  NString str = cast<NString>(str_val);
  CHECK_EQ(knirk.length(), str.length());
  const char *c_str = str.c_str();
  for (int i = 0; i < str.length(); i++) {
    CHECK_EQ(knirk[i], str[i]);
    CHECK_EQ(knirk[i], c_str[i]);
  }
  CHECK_EQ(0, c_str[str.length()]);
  
  NValue emp_val = ApiUtils::new_value(dict, f_emp);
  CHECK_EQ(vtTuple, emp_val.type());
  CHECK(!is<NInteger>(emp_val));
  CHECK(!is<NString>(emp_val));
  CHECK(is<NTuple>(emp_val));
  NTuple emp = cast<NTuple>(emp_val);
  CHECK_EQ(0, emp.length());
}
