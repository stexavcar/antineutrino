#ifndef _POINTERS
#define _POINTERS

#include "utils/checks.h"
#include "utils/types.h"

namespace neutrino {

/**
 * Contains all the methods for manipulating direct object pointers,
 * including tagging/untagging and accessing object fields.
 */
class ValuePointer {
public:
  
  /**
   * Converts an object pointer into a direct C pointer to the
   * beginning address of the object in the heap.
   */
  static inline address address_of(void *obj);
  
  /**
   * Converts an object pointer into an offset within a buffer.
   */
  static inline uword offset_of(void *obj);

  /**
   * Converts the heap address of an object to a tagged object pointer.
   */
  static inline Object *tag_as_object(address addr);
  
  static inline uword tag_as_object(word *addr);

  static inline bool has_object_tag(void *val);
  
  static inline bool has_smi_tag(void *val);
  
  /**
   * Converts this literal integer into a tagged small integer.  The
   * value must not require more than 31 bits to represent.
   */
  static inline Smi *tag_as_smi(word val);
  
  /**
   * Returns the value of a tagged small integer.
   */
  static inline word value_of(void *val);
  
  static inline bool is_valid_smi(word val);
  
  static inline Forwarder *tag_as_forwarder(address val);
  
  static inline bool has_forwarder_tag(void *val);
  
  static inline address target_of(void *forwarder);
  
  /**
   * Packages the given type and payload into a signal-tagged value.
   * The type must not require more then kSignalTypeSize bits and the
   * payload not more than kSignalPayloadSize bits.
   */
  static inline Signal *tag_as_signal(uword type, uword payload);

  static inline uword tag_as_signal(address addr);
  static inline uword un_signal_tag(void *value);
  
  static inline bool has_signal_tag(void *val);
  
  /**
   * Masks out and returns the type field of a signal.
   */
  static inline uword signal_type(Signal *val);
  
  /**
   * Masks out and returns the payload field of a signal.
   */
  static inline uword signal_payload(Signal *val);

  static inline bool is_aligned(uword size);
  
  /**
   * Returns the smalles aligned value greater than the given value.
   */
  static inline uword align(uword size);

  template <typename T>
  static inline void set_field(Object *obj, uword offset, T value);
  
  /**
   * Accesses the field at the specified offset, viewed as the
   * specified type.  The offset is counted in bytes, which must be
   * word size aligned, and is not affected by the type under which the
   * result is viewed.
   */
  template <typename T>
  static inline T &access_field(Object *obj, uword offset);
  
  /**
   * Accesses the field at the specified offset, viewed as the
   * specified type.  The offset is counted in bytes and is not affected
   * by the type under which the result is viewed.
   */
  template <typename T>
  static inline T &access_direct(Object *obj, uword offset);
  
  static const uword kUninitialized = 0xFeedDead;

private:
  
#define MASK(value) ((1 << value) - 1)

  static const uword kSmiTagSize          = 2;
  static const uword kObjectTagSize       = 2;
  static const uword kForwarderTagSize    = 2;
  static const uword kSignalTypeSize      = 2;
  static const uword kSignalPayloadSize   = 28;
  
  static const uword kObjectAlignmentSize = kObjectTagSize;
  static const uword kObjectAlignmentMask = MASK(kObjectAlignmentSize);

  static const uword kObjectTagMask       = MASK(kObjectTagSize);
  static const uword kForwarderTagMask    = MASK(kForwarderTagSize);
  static const uword kSmiTagMask          = MASK(kSmiTagSize);
  static const uword kSignalTypeMask      = MASK(kSignalTypeSize);
  static const uword kSignalPayloadMask   = MASK(kSignalPayloadSize);
  
  static const uword kSmiTag              = 0x0;
  static const uword kObjectTag           = 0x1;
  static const uword kForwarderTag        = 0x2;
  static const uword kSignalTag           = 0x3;

#undef MASK

};

} // namespace neutrino

#endif // _POINTERS
