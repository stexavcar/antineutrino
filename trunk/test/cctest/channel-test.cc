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
  IValue value = *((IValue*) 0);
  is<IString>(value);
  is<ITuple>(value);
  cast<IString>(value);
  cast<ITuple>(value);
}

void Test::simple_objects() {
  LocalRuntime runtime;
  Smi *smi_obj = Smi::from_int(10);
  string knirk = "knirk";
  String *str_obj = cast<String>(runtime.heap().new_string(knirk));
  Tuple *emp_obj = cast<Tuple>(runtime.heap().new_tuple(0));

  ImageOutputStream stream;
  RawFValue *raw_smi = stream.marshal(smi_obj);
  RawFValue *raw_str = stream.marshal(str_obj);
  RawFValue *raw_emp = stream.marshal(emp_obj);
  FrozenHeap heap(stream);
  FImmediate *f_smi = heap.cook(raw_smi);
  FImmediate *f_str = heap.cook(raw_str);
  FImmediate *f_emp = heap.cook(raw_emp);
  MethodDictionaryImpl dict;
  
  IValue smi_val = dict.new_value(f_smi);
  CHECK_EQ(vtInteger, smi_val.type());
  CHECK(is<IInteger>(smi_val));
  CHECK(!is<IString>(smi_val));
  CHECK(!is<ITuple>(smi_val));
  IInteger smi = cast<IInteger>(smi_val);
  CHECK_EQ(10, smi.value());
  
  IValue str_val = dict.new_value(f_str);
  CHECK_EQ(vtString, str_val.type());
  CHECK(!is<IInteger>(str_val));
  CHECK(is<IString>(str_val));
  CHECK(!is<ITuple>(str_val));
  IString str = cast<IString>(str_val);
  CHECK_EQ(knirk.length(), str.length());
  const char *c_str = str.c_str();
  for (int i = 0; i < str.length(); i++) {
    CHECK_EQ(knirk[i], str[i]);
    CHECK_EQ(knirk[i], c_str[i]);
  }
  CHECK_EQ(0, c_str[str.length()]);
  
  IValue emp_val = dict.new_value(f_emp);
  CHECK_EQ(vtTuple, emp_val.type());
  CHECK(!is<IInteger>(emp_val));
  CHECK(!is<IString>(emp_val));
  CHECK(is<ITuple>(emp_val));
  ITuple emp = cast<ITuple>(emp_val);
  CHECK_EQ(0, emp.length());
}
