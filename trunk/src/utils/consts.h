#ifndef _HEAP_CONSTS
#define _HEAP_CONSTS

// ---------------------------
// --- C l a s s   T a g s ---
// ---------------------------

#define FOR_EACH_OBJECT_TYPE(VISIT)                                  \
  VISIT(1,  CLASS, Class)                                            \
  VISIT(2,  STRING, String)                                          \
  VISIT(3,  TUPLE, Tuple)                                            \
  VISIT(4,  DICTIONARY, Dictionary)                                  \
  VISIT(5,  VOID, Void)                                              \
  VISIT(6,  NULL, Null)                                              \
  VISIT(7,  TRUE, True)                                              \
  VISIT(8,  FALSE, False)                                            \
  VISIT(9,  LAMBDA, Lambda)                                          \
  VISIT(10, BUFFER, Buffer)                                          \
  VISIT(11, CODE, Code)                                              \
  VISIT(12, METHOD, Method)                                          \
  VISIT(13, PROTOCOL, Protocol)                                      \
  FOR_EACH_SYNTAX_TREE_TYPE(VISIT)

#define FOR_EACH_VIRTUAL_TYPE(VISIT)                                 \
  VISIT(20, OBJECT, Object)                                          \
  VISIT(21, VALUE, Value)                                            \
  VISIT(22, ABSTRACT_BUFFER, AbstractBuffer)

#define FOR_EACH_SIGNAL_TYPE(VISIT)                                  \
  VISIT(30, ALLOCATION_FAILED, AllocationFailed)                     \
  VISIT(31, INTERNAL_ERROR, InternalError)                           \
  VISIT(32, NOTHING, Nothing)                                        \
  VISIT(33, PENDING_REGISTER, PendingRegister)

#define FOR_EACH_VALUE_TYPE(VISIT)                                   \
  VISIT(40, SMI, Smi)                                                \
  VISIT(41, SIGNAL, Signal)                                          \
  FOR_EACH_SIGNAL_TYPE(VISIT)                                        \
  FOR_EACH_OBJECT_TYPE(VISIT)

#define FOR_EACH_DECLARED_TYPE(VISIT)                                \
  FOR_EACH_VALUE_TYPE(VISIT)                                         \
  FOR_EACH_VIRTUAL_TYPE(VISIT)

#define FOR_EACH_SYNTAX_TREE_TYPE(VISIT)                             \
  VISIT(50, LITERAL, Literal)


// ---------------------
// --- O p c o d e s ---
// ---------------------

#define FOR_EACH_OPCODE(VISIT)                                       \
  VISIT(0,  PUSH,  1)   VISIT(1,  RETURN, 0) VISIT(2,  GLOBAL,   1)  \
  VISIT(3,  CALL,  1)   VISIT(4,  SLAP,   1) VISIT(5,  ARGUMENT, 1)  \
  VISIT(6,  VOID,  0)   VISIT(7,  NUHLL,  0) VISIT(8,  TRUE,     0)  \
  VISIT(9,  FALSE, 0)   VISIT(10, POP,    1) VISIT(11, IF_TRUE,  1)  \
  VISIT(12, GOTO,  1)   VISIT(13, INVOKE, 2) VISIT(14, INTERNAL, 2)


// -------------------------------------------
// --- I m a g e   I n s t r u c t i o n s ---
// -------------------------------------------

#define FOR_EACH_IMAGE_OBJECT_CONST(VISIT)                           \
  VISIT(0, Object, TypeOffset)     VISIT(1, Object, HeaderSize)      \
  VISIT(2, Dictionary, Size)                                         \
  VISIT(1, String, LengthOffset)   VISIT(2, String, HeaderSize)      \
  VISIT(1, Code, LengthOffset)     VISIT(2, Code, HeaderSize)        \
  VISIT(1, Tuple, LengthOffset)    VISIT(2, Tuple, HeaderSize)       \
  VISIT(1, Lambda, ArgcOffset)     VISIT(2, Lambda, CodeOffset)      \
  VISIT(3, Lambda, LiteralsOffset) VISIT(4, Lambda, Size)


// -----------------
// --- R o o t s ---
// -----------------

#define FOR_EACH_SIMPLE_ROOT(VISIT)                                  \
  VISIT(0,  Class, string_class, STRING_CLASS, new_class(STRING_TYPE))\
  VISIT(1,  Class, tuple_class, TUPLE_CLASS, new_class(TUPLE_TYPE))  \
  VISIT(2,  Class, void_class, VOID_CLASS, new_class(VOID_TYPE))     \
  VISIT(3,  Void, vhoid, VOID, new_singleton(void_class()))          \
  VISIT(4,  Class, null_class, NULL_CLASS, new_class(NULL_TYPE))     \
  VISIT(5,  Null, nuhll, NULL, new_singleton(null_class()))          \
  VISIT(6,  Class, true_class, TRUE_CLASS, new_class(TRUE_TYPE))     \
  VISIT(7,  True, thrue, TRUE, new_singleton(true_class()))          \
  VISIT(8,  Class, false_class, FALSE_CLASS, new_class(FALSE_TYPE))  \
  VISIT(9, False, fahlse, FALSE, new_singleton(false_class()))      \
  VISIT(10, Class, literal_class, LITERAL_CLASS, new_class(LITERAL_TYPE))\
  VISIT(11, Class, dictionary_class, DICTIONARY_CLASS, new_class(DICTIONARY_TYPE))\
  VISIT(12, Class, lambda_class, LAMBDA_CLASS, new_class(LAMBDA_TYPE))\
  VISIT(13, Dictionary, toplevel, TOPLEVEL, new_dictionary())        \
  VISIT(14, Class, buffer_class, BUFFER_CLASS, new_class(BUFFER_TYPE))\
  VISIT(15, Class, code_class, CODE_CLASS, new_class(CODE_TYPE))     \
  VISIT(16, Class, method_class, METHOD_CLASS, new_class(METHOD_TYPE))\
  VISIT(17, Class, smi_class, SMI_CLASS, new_class(SMI_TYPE))

#define FOR_EACH_COMPLICATED_ROOT(VISIT)                             \
  VISIT(18, Class, class_class, CLASS_CLASS, NULL)

#define FOR_EACH_ROOT(VISIT)                                         \
  FOR_EACH_COMPLICATED_ROOT(VISIT)                                   \
  FOR_EACH_SIMPLE_ROOT(VISIT)


// ---------------------------------------
// --- B u i l t - i n   C l a s s e s ---
// ---------------------------------------

#define FOR_EACH_BUILTIN_CLASS(VISIT)                                \
  VISIT(0, SmallInteger, smi,  SMI)  VISIT(1, String, string, STRING)      \
  VISIT(2, True,         true, TRUE) VISIT(3, False,  false,  FALSE)       \
  VISIT(4, Void,         void, VOID) VISIT(5, Null,   null,   NULL)

#endif // _HEAP_CONSTS
