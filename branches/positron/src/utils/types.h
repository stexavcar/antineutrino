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

typedef uint16_t code_t;

namespace neutrino {

template <typename T> class allocation;
class Arena;
class CodeStream;
class DisassembleContext;
class Factory;
class FieldMigrator;
class ForwardPointer;
template <typename T> class likely;
class MiniHeap;
class MessageIn;
class MessageOut;
class pattern;
template <typename T> class persistent;
class persistent_cell;
class possibly;
template <typename T> class ref;
class RefManager;
class Runtime;
class Space;
class Species;
class string;
class string_stream;
class s_exp;
class Value;
class variant_type;
template <typename T> class variant_type_impl;

namespace plankton {

class Array;
class DTable;
class Integer;
class MessageData;
class Null;
class Object;
class Pattern;
class Seed;
class String;

} // namespace plankton

} // namespace neutrino


#endif // _UTILS_TYPES
