#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "public/plankton.h"

static void test_foo() {
  static const char *kTestString = "nirk";
  unsigned i;
  unsigned found;
  unsigned test_string_length = strlen(kTestString);
  plankton_value_t int_value, str_value;
  plankton_value_factory_t factory = plankton_new_value_factory();
  
  int_value = plankton_new_integer(factory, 10);
  assert(int_value.dtable->type(int_value) == PLANKTON_INTEGER);
  assert(int_value.dtable->integer_value(int_value) == 10);
    
  str_value = plankton_new_string(factory, kTestString, test_string_length);
  assert(str_value.dtable->type(str_value) == PLANKTON_STRING);
  assert(str_value.dtable->string_length(str_value) == test_string_length);
  for (i = 0; i < test_string_length; i++) {
    found = str_value.dtable->string_char_at(str_value, i);
    assert(found == kTestString[i]);
  }  
  plankton_dispose_value_factory(factory);
}
