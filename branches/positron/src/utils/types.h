#ifndef _UTILS_TYPES
#define _UTILS_TYPES


#ifdef MSVC
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif


namespace positron {


class MessageBuffer;
class p_array;
class p_integer;
class p_null;
class p_string;


} // namespace positron


#endif // _UTILS_TYPES
