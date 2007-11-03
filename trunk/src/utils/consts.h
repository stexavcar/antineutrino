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
  VISIT(32, NOTHING, Nothing)

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
  VISIT(PUSH,  0,  1)   VISIT(RETURN,  1,  0) VISIT(GLOBAL,   2,  1) \
  VISIT(CALL,  3,  1)   VISIT(SLAP,    4,  1) VISIT(ARGUMENT, 5,  1) \
  VISIT(VOID,  6,  0)   VISIT(NUHLL,   7,  0) VISIT(TRUE,     8,  0) \
  VISIT(FALSE, 9,  0)   VISIT(POP,    10,  1) VISIT(IF_TRUE, 11, 1)  \
  VISIT(GOTO,  12, 1)   VISIT(INVOKE, 13, 2)  VISIT(INTERNAL, 14, 2)


// -------------------------------------------
// --- I m a g e   I n s t r u c t i o n s ---
// -------------------------------------------

#define FOR_EACH_INSTRUCTION(VISIT)                                  \
  VISIT(1,  SET_DICTIONARY_CLASS) VISIT(2,  SET_TOPLEVEL)            \
  VISIT(3,  SET_CLASS_CLASS)      VISIT(4,  SET_LAMBDA_CLASS)        \
  VISIT(5,  SET_TUPLE_CLASS)      VISIT(6,  SET_CODE_CLASS)          \
  VISIT(7,  SET_STRING_CLASS)                                        \
  VISIT(20, NEW_DICTIONARY)       VISIT(21, NEW_STRING)              \
  VISIT(22, NEW_LAMBDA)           VISIT(23, NEW_CODE)                \
  VISIT(24, NEW_TUPLE)            VISIT(25, NEW_CLASS)               \
  VISIT(40, LOAD_REGISTER)

#endif // _HEAP_CONSTS
