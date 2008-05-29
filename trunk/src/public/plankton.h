#ifndef _PUBLIC_PLANKTON_H
#define _PUBLIC_PLANKTON_H

typedef enum {
  PLANKTON_INTEGER, PLANKTON_UNKNOWN, PLANKTON_STRING
} plankton_value_tag_t;

struct _plankton_dtable_t;
struct _plankton_value_factory_impl_t;

/* A plankton value. */
typedef struct {
  struct _plankton_dtable_t *dtable;
  void *origin;
} plankton_value_t;

/* A d-table, the set of functions to use together with a value. */
typedef struct _plankton_dtable_t {
  plankton_value_tag_t (*type)(plankton_value_t value);
  int (*integer_value)(plankton_value_t value);
  unsigned (*string_length)(plankton_value_t value);
  unsigned (*string_char_at)(plankton_value_t value, unsigned index);
} plankton_dtable_t;

/* A plankton value factory. */
typedef struct {
  struct _plankton_value_factory_impl_t *impl;
} plankton_value_factory_t;

/* Creates a new value factory. */
plankton_value_factory_t plankton_new_value_factory();

/* Disposes a value factory.  All values allocated using the factory
 * will be freed by this operation. */
void plankton_dispose_value_factory(plankton_value_factory_t value);

/* Creates a new plankton integer value. */
plankton_value_t plankton_new_integer(plankton_value_factory_t factory,
    int value);

/* Creates a new plankton string value. */
plankton_value_t plankton_new_string(plankton_value_factory_t factory,
    const char *str, unsigned length);

#endif /* _PUBLIC_PLANKTON_H */
