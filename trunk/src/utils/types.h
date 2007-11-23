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
class Heap;
class Image;
class Lambda;
class LambdaExpression;
class Object;
template <class T> class ref;
class Roots;
class Runtime;
class string_buffer;
class Value;
class Visitor;
class String;
class Symbol;
class SyntaxTree;
class Tuple;

class ImageTuple;

class AbstractEnumInfo;

}

#endif // _TYPES
