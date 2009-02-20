#ifndef _VALUE_POINTER
#define _VALUE_POINTER

#include "value/value.h"

namespace neutrino {

class Pointer {
public:
  static inline Signal *tag_signal(Signal::Type type, word payload);
  static inline bool is_signal(Data *data);
  static inline Signal::Type signal_type(Signal *that);
  static inline word signal_payload(Signal *that);

  static inline ForwardPointer *tag_forward_pointer(Object *value);
  static inline bool is_forward_pointer(Data *data);
  static inline Object *forward_pointer_target(ForwardPointer *that);

  static inline Object *as_object(uint8_t *addr);

  static inline bool is_object(Data *data);
private:
  static const uword kTagSize = 2;
  static const uword kTagMask = (1 << kTagSize) - 1;
  static const uword kObjectTag = 0;

  static const uword kSignalTag = 3;
  static const uword kForwardPointerTag = kSignalTag;
  static const uword kSignalPayloadSize = 10;
};

} // namespace neutrino

#endif // _VALUE_POINTER
