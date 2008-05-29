#include "public/plankton.h"

#include <stdlib.h>
#include <string.h>


#define NEW_ARRAY(type_t, n) ((type_t*) malloc(sizeof(type_t) * n))
#define NEW(type_t)          NEW_ARRAY(type_t, 1)


static unsigned kTagMask = 0x3;
static unsigned kSmiTag = 0x0;
static unsigned kObjectTag = 0x1;


/**
 * The value factory implementation.
 */
typedef struct _plankton_value_factory_impl_t {
  plankton_dtable_t value_dtable;
  unsigned size;
  unsigned capacity;
  unsigned *buffer;
} plankton_value_factory_impl_t;


static plankton_value_tag_t dtable_type(plankton_value_t value) {
  plankton_value_factory_impl_t *impl;
  int origin = (int) value.origin;
  /* If the origin is smi tagged then it must be a smi. */
  if ((origin & kTagMask) == kSmiTag) return PLANKTON_INTEGER;
  /* Otherwise we need to look more closely at the associated memory. */
  impl = (plankton_value_factory_impl_t*) value.dtable;
  return impl->buffer[origin >> 2];
}


static int dtable_integer_value(plankton_value_t value) {
  return ((int) value.origin) >> 2;
}


static unsigned dtable_string_length(plankton_value_t value) {
  plankton_value_factory_impl_t *impl;
  int offset = ((int) value.origin) >> 2;
  impl = (plankton_value_factory_impl_t*) value.dtable;
  return impl->buffer[offset + 1];
}


static unsigned dtable_string_char_at(plankton_value_t value, unsigned index) {
  plankton_value_factory_impl_t *impl;
  int offset = ((int) value.origin) >> 2;
  impl = (plankton_value_factory_impl_t*) value.dtable;
  return impl->buffer[offset + 2 + index];
}


static void initialize_dtable(plankton_dtable_t *dtable) {
  dtable->type = dtable_type;
  dtable->integer_value = dtable_integer_value;
  dtable->string_length = dtable_string_length;
  dtable->string_char_at = dtable_string_char_at;
}


plankton_value_factory_t plankton_new_value_factory() {
  plankton_value_factory_impl_t *impl = NEW(plankton_value_factory_impl_t);
  plankton_value_factory_t result;
  initialize_dtable(&impl->value_dtable);
  impl->size = 0;
  impl->capacity = 16;
  impl->buffer = NEW_ARRAY(unsigned, 16);
  result.impl = impl;
  return result;
}


void plankton_dispose_value_factory(plankton_value_factory_t value) {
  free(value.impl->buffer);
  free(value.impl);
}


plankton_value_t plankton_new_integer(plankton_value_factory_t factory,
    int value) {
  plankton_value_t result;
  result.dtable = &factory.impl->value_dtable;
  result.origin = (void*) ((value << 2) | kSmiTag);
  return result;
}


/**
 * Allocates a new heap object using this factory.
 */
static plankton_value_t new_heap_object(plankton_value_factory_t factory,
    plankton_value_tag_t tag, unsigned size, unsigned **memory) {
  plankton_value_factory_impl_t *impl = factory.impl;
  plankton_value_t result;
  unsigned new_capacity;
  unsigned *new_buffer;
  unsigned *allocated;
  /* Make sure there's room enough in the buffer */
  unsigned target_offset = impl->size + size + 1;
  if (target_offset >= impl->capacity) {
    new_capacity = 2 * target_offset;
    new_buffer = NEW_ARRAY(unsigned, new_capacity);
    memcpy(new_buffer, impl->buffer, impl->size * sizeof(unsigned));
    free(impl->buffer);
    impl->capacity = new_capacity;
    impl->buffer = new_buffer;
  }
  /* Set up the result value */
  result.dtable = &impl->value_dtable;
  result.origin = (void*) ((impl->size << 2) | kObjectTag);
  /* Initialize the memory area just allocated and update impl */
  allocated = impl->buffer + impl->size;
  allocated[0] = tag;
  *memory = allocated + 1;
  impl->size = target_offset;
  return result;
}


plankton_value_t plankton_new_string(plankton_value_factory_t factory,
    const char *str, unsigned length) {
  unsigned total_size = length + 1;
  unsigned *memory = NULL;
  unsigned i;
  plankton_value_t result = new_heap_object(factory, PLANKTON_STRING,
      total_size, &memory);
  memory[0] = length;
  for (i = 0; i < length; i++)
    memory[i + 1] = str[i];
  return result;
}
