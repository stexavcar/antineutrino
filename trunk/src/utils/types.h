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
class AllocationFailed;
class Architecture;
class Arguments;
template <class Config> class Assembler;
class Layout;
class LocalVariable;
class Code;
class CodeGenerator;
class Context;
class Data;
class HashMap;
class DoClause;
class DoOnExpression;
class ExtendedValueDTable;
class Factory;
class False;
class FatalError;
class ForwarderDescriptor;
class GarbageCollectionMonitor;
class Heap;
class Image;
class ImageContext;
class ImageLoadStatus;
class FImmediate;
class Immediate;
class Instance;
class InterpreterState;
class Lambda;
class LambdaExpression;
class LiteralExpression;
class Method;
class Mirror;
class Null;
class Object;
class Parameters;
template <class T> class persistent;
class persistent_cell;
class QuoteTemplate;
class RawFValue;
template <class T> class ref;
class RefManager;
class ReturnExpression;
class Root;
class Roots;
class Runtime;
class Scope;
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
class Signal;
class String;
class Symbol;
class SyntaxTree;
class Tuple;

class FTuple;

class AbstractEnumInfo;


// -------------------
// --- O p t i o n ---
// -------------------


template <class Type, class Failure = Signal>
class maybe {
public:
  inline maybe(Type *value);
  inline maybe(Failure *failure);
  template <class T, class F>
  inline maybe(const maybe<T, F> &other) : data_(other.data()) {
    TYPE_CHECK(Type*, T*);
    TYPE_CHECK(Failure*, F*);
  }
  inline bool has_failed();
  inline Type *value();
  inline Failure *signal();
  inline Data *data() const { return data_; }
private:
  Data *data_;
};


template <class T>
class allocation : public maybe<T, AllocationFailed> {
public:
  allocation(T *value) : maybe<T, AllocationFailed>(value) { }
  allocation(AllocationFailed *value) : maybe<T, AllocationFailed>(value) { }
};


template <class T>
class probably : public maybe<T, FatalError> {
public:
  probably(T *value) : maybe<T, FatalError>(value) { }
  probably(FatalError *value) : maybe<T, FatalError>(value) { }
};


#define KLIDKIKS_CHECK_OBJ(__Type__, __name__, __operation__)        \
  maybe<__Type__> __##__name__##_opt__ = __operation__;              \
  if (__##__name__##_opt__.has_failed()) return __##__name__##_opt__.signal(); \
  __Type__* __name__ = cast<__Type__>(__##__name__##_opt__.value());


#define KLIDKIKS_ALLOC(__Type__, __name__, __operation__)            \
  allocation<__Type__> __##__name__##_alloc__ = __operation__;       \
  if (__##__name__##_alloc__.has_failed()) return __##__name__##_alloc__.signal(); \
  __Type__ *__name__ = __##__name__##_alloc__.value();


#define KLIDKIKS_OBJALLOC(__Type__, __name__, __operation__)         \
  allocation<Object> __##__name__##_alloc__ = __operation__;         \
  if (__##__name__##_alloc__.has_failed()) return __##__name__##_alloc__.signal(); \
  __Type__ *__name__ = cast<__Type__>(__##__name__##_alloc__.value());


#define KLIDKIKS_BUFALLOC(__Type__, __name__, __operation__)         \
  allocation<AbstractBuffer> __##__name__##_alloc__ = __operation__; \
  if (__##__name__##_alloc__.has_failed()) return __##__name__##_alloc__.signal(); \
  __Type__ *__name__ = cast<__Type__>(__##__name__##_alloc__.value());


} // neutrino

#endif // _TYPES
