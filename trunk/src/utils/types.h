#ifndef _TYPES
#define _TYPES

#ifdef GCC
#include <stdint.h>
#endif

#ifdef MSVC
#ifdef M32
typedef long long uint64_t;
typedef unsigned long long int64_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned char uint8_t;
typedef signed char int8_t;
#endif
#endif

typedef int8_t byte;
typedef byte* address;

#if defined(M32)
typedef int32_t word;
typedef uint32_t uword;
#elif defined(M64)
typedef int64_t word;
typedef uint64_t uword;
#endif

#define TYPE_CHECK(S, T) while (false) { *(static_cast<S*>(0)) = static_cast<T>(0); }

template <typename T>
static inline T function_cast(void *obj) {
  return reinterpret_cast<T>(reinterpret_cast<uword>(obj));
}

template <typename T>
class TypeConsts {
};

template <>
class TypeConsts<uword> {
public:
  static const uword kMax = ~0;
};

template <>
class TypeConsts<uint16_t> {
public:
  static const uint16_t kMax = ~0;
};

// Forward declarations
namespace neutrino {

class AbstractWatch;
class Arguments;
template <class Config> class Assembler;
class Layout;
class Code;
class CodeGenerator;
class Context;
class Data;
class Dictionary;
class DoClause;
class DoOnExpression;
class ExtendedValueDTable;
class Factory;
class False;
class ForwarderDescriptor;
class GarbageCollectionMonitor;
class Heap;
class Image;
class ImageContext;
class ImageLoadStatus;
class FImmediate;
class Immediate;
class Instance;
class Lambda;
class LambdaExpression;
class LiteralExpression;
class Method;
class Mirror;
class Null;
class Object;
class Parameters;
template <class T> class persistent;
class QuoteTemplate;
class RawFValue;
template <class T> class ref;
class RefStack;
class ReturnExpression;
class Root;
class Roots;
class Runtime;
class Selector;
class SemiSpace;
class Signature;
class Smi;
class Stack;
class StackFlags;
class string_buffer;
class Task;
class True;
class Value;
class Visitor;
class Void;
class String;
class Symbol;
class SyntaxTree;
class Tuple;

class FTuple;

class AbstractEnumInfo;

}

#endif // _TYPES
