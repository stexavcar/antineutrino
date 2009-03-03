#ifndef _VALUE_POINTER
#define _VALUE_POINTER

#include "utils/global.h"

namespace neutrino {

class Pointer {
public:
  static inline Signal *tag_signal(word type, word payload);
  static inline bool is_signal(Data *data);
  static inline word signal_type(Signal *that);
  static inline word signal_payload(Signal *that);

  static inline bool fits_tagged_integer(word value);
  static inline TaggedInteger *tag_integer(word value);
  static inline bool is_tagged_integer(Data *data);
  static inline word tagged_integer_value(TaggedInteger *that);

  static inline ForwardPointer *tag_forward_pointer(Object *value);
  static inline bool is_forward_pointer(Data *data);
  static inline Object *forward_pointer_target(ForwardPointer *that);

  static inline Object *as_object(uint8_t *addr);

  static inline bool fits_small_double(double value);
  static inline bool is_small_double(Data *data);
  static inline SmallDouble *tag_small_double(double value);
  static inline double untag_small_double(SmallDouble *that);

  static inline bool is_object(Data *data);

  static const uword kTagSize = IF_ELSE(cc32, 2, 3);
  static const word kBitsPerWord = sizeof(word) * 8;
  static const word kTaggedIntegerUpperLimit = 1L << (kBitsPerWord - kTagSize - 1);
  static const word kTaggedIntegerLowerLimit = -kTaggedIntegerUpperLimit - 1;

private:
  static const uword kTagMask = (1 << kTagSize) - 1;
  static const uword kObjectTag = 0;
  static const uword kSmadTag = 2;

  static const uword kIntegerTag = 1;

  static const uword kSignalTag = 3;
  static const uword kForwardPointerTag = kSignalTag;
  static const uword kSignalPayloadSize = 10;

#ifdef M64
  static const uword kDoubleExponentBias = 1023;
  static const uword kDoubleMantissaSize = 52;
  static const uword kDoubleExponentSize = 11;

  static const uword kExponentRange = 127;
  static const uword kStolenDoubleBitsCount = 3;
  static const uword kStolenDoubleBitsStart = 63 - kStolenDoubleBitsCount;
  static const uword kDoubleBias = (kDoubleExponentBias - kExponentRange) << kDoubleMantissaSize;
  static const uword kStolenDoubleBitsMask =
    static_cast<uword>((1 << kStolenDoubleBitsCount) - 1) << kStolenDoubleBitsStart;
#endif

};

} // namespace neutrino

#endif // _VALUE_POINTER
