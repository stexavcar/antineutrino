#ifndef _POINTERS
#define _POINTERS

#include "heap/values.h"
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
  static inline uint32_t offset_of(void *obj);

  /**
   * Converts the heap address of an object to a tagged object pointer.
   */
  static inline Object *tag_as_object(address addr);
  
  static inline uint32_t tag_offset_as_object(uint32_t value);

  static inline bool has_object_tag(void *val);
  
  static inline bool has_smi_tag(void *val);
  
  /**
   * Converts this literal integer into a tagged small integer.  The
   * value must not require more than 31 bits to represent.
   */
  static inline Smi *tag_as_smi(int32_t val);
  
  /**
   * Returns the value of a tagged small integer.
   */
  static inline int32_t value_of(void *val);
  
  static inline bool is_valid_smi(int32_t val);
  
  /**
   * Packages the given type and payload into a signal-tagged value.
   * The type must not require more then kSignalTypeSize bits and the
   * payload not more than kSignalPayloadSize bits.
   */
  static inline Signal *tag_as_signal(uint32_t type, uint32_t payload);

  static inline uint32_t tag_as_signal(address addr);
  static inline uint32_t un_signal_tag(void *value);
  
  static inline bool has_signal_tag(void *val);
  
  /**
   * Masks out and returns the type field of a signal.
   */
  static inline uint32_t signal_type(Signal *val);
  
  /**
   * Masks out and returns the payload field of a signal.
   */
  static inline uint32_t signal_payload(Signal *val);

  static inline bool is_aligned(uint32_t size);
  
  /**
   * Returns the smalles aligned value greater than the given value.
   */
  static inline uint32_t align(uint32_t size);

  template <typename T>
  static inline void set_field(Object *obj, uint32_t offset, T value);
  
  /**
   * Accesses the field at the specified offset, viewed as the
   * specified type.  The offset is counted in bytes, which must be
   * word size aligned, and is not affected by the type under which the
   * result is viewed.
   */
  template <typename T>
  static inline T &access_field(Object *obj, uint32_t offset);
  
  /**
   * Accesses the field at the specified offset, viewed as the
   * specified type.  The offset is counted in bytes and is not affected
   * by the type under which the result is viewed.
   */
  template <typename T>
  static inline T &access_direct(Object *obj, uint32_t offset);
  
  static const uint32_t kUninitialized = 0xFeedDead;

private:
  
#define MASK(value) ((1 << value) - 1)

  static const uint32_t kSmiTagSize          = 1;
  static const uint32_t kObjectTagSize       = 2;
  static const uint32_t kSignalTypeSize      = 2;
  static const uint32_t kSignalPayloadSize   = 28;
  
  static const uint32_t kObjectAlignmentSize = kObjectTagSize;
  static const uint32_t kObjectAlignmentMask = MASK(kObjectAlignmentSize);

  static const uint32_t kObjectTagMask       = MASK(kObjectTagSize);
  static const uint32_t kSmiTagMask          = MASK(kSmiTagSize);
  static const uint32_t kSignalTypeMask      = MASK(kSignalTypeSize);
  static const uint32_t kSignalPayloadMask   = MASK(kSignalPayloadSize);
  
  static const uint32_t kObjectTag           = 0x1;
  static const uint32_t kSmiTag              = 0x0;
  static const uint32_t kSignalTag           = 0x3;

#undef MASK

};

} // namespace neutrino

#endif // _POINTERS
