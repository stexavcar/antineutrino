#ifndef _TYPES
#define _TYPES

#include <stdint.h>

typedef int32_t word;
typedef int8_t* address;

#define TYPE_CHECK(S, T) while (false) { *(static_cast<S*>(0)) = static_cast<T>(0); }

// Forward declarations
namespace neutrino {

class Class;
class Code;
class Data;
class Dictionary;
class Factory;
class False;
class Heap;
class Image;
class Instance;
class Lambda;
class LambdaExpression;
class Method;
class Null;
class Object;
template <class T> class persistent;
class QuoteTemplate;
template <class T> class ref;
class ReturnExpression;
class Roots;
class Runtime;
class SemiSpace;
class Smi;
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
