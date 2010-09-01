#ifndef _PLANKTON_CHANNEL_H
#define _PLANKTON_CHANNEL_H

#include "plankton/c/plankton.h"

typedef struct _plankton_channel_impl_t plankton_channel_impl_t;

typedef struct _plankton_channel_dtable_t {
  void (*send)(plankton_value_t);
} plankton_channel_dtable_t;

typedef struct _plankton_channel_t {
  plankton_channel_dtable_t *dtable;
  plankton_channel_impl_t *impl;
} plankton_channel_t;

plankton_channel_t plankton_has_parent();
void plankton_dispose_channel(plankton_channel_t channel);

#endif /* _PLANKTON_CHANNEL_H */
