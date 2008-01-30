#ifndef _TYPES
#define _TYPES

#ifdef GCC
#include <stdint.h>
#endif

#ifdef MSVC
typedef unsigned int uint32_t;
typedef signed int int32_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned char uint8_t;
typedef signed char int8_t;
#endif

typedef int8_t byte;
typedef int32_t word;
typedef byte* address;

#define TYPE_CHECK(S, T) while (false) { *(static_cast<S*>(0)) = static_cast<T>(0); }

// Forward declarations
namespace neutrino {

class AbstractWatch;
class Assembler;
class Layout;
class Code;
class Context;
class Data;
class Dictionary;
class DoClause;
class DoOnExpression;
class Factory;
class False;
class GarbageCollectionMonitor;
class Heap;
class Image;
class Instance;
class Lambda;
class LambdaExpression;
class LiteralExpression;
class Method;
class Mirror;
class Null;
class Object;
template <class T> class persistent;
class QuoteTemplate;
template <class T> class ref;
class ReturnExpression;
class Roots;
class Runtime;
class SemiSpace;
class Signature;
class Smi;
class Stack;
class StackFlags;
class string_buffer;
class True;
class Value;
class Visitor;
class Void;
class String;
class Symbol;
class SyntaxTree;
class Tuple;

class ImageTuple;

class AbstractEnumInfo;

}

#endif // _TYPES
