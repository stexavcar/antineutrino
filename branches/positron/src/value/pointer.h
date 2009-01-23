#ifndef _VALUE_POINTER
#define _VALUE_POINTER

#include "value/value.h"

namespace positron {

class Pointer {
public:
  static inline Signal *tag_signal(Signal::Type type);
  static inline bool is_signal(Data *data);
  static inline Signal::Type signal_type(Signal *that);
private:
  static const uword kTagSize = 2;
  static const uword kTagMask = (1 << kTagSize) - 1;
  static const uword kSignalTag = 3;
};

} // namespace positron

#endif // _VALUE_POINTER
