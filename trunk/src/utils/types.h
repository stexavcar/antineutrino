#ifndef _TYPES
#define _TYPES

#include <sys/types.h>

typedef int32_t word;
typedef int8_t* address;

#define TYPE_CHECK(S, T) while (false) { *(static_cast<S*>(0)) = static_cast<T>(0); }

// Forward declarations
namespace neutrino {

class Code;
class Data;
class Dictionary;
class Heap;
class Lambda;
class Object;
template <class T> class ref;
class Roots;
class Runtime;
class string_buffer;
class Class;
class Value;
class String;
class SyntaxTree;
class Tuple;

class AbstractEnumInfo;

}

#endif // _TYPES
