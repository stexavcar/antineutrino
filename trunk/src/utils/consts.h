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

#define FOR_EACH_INSTRUCTION(VISIT)                                  \
  VISIT(1,  SET_DICTIONARY_CLASS) VISIT(2,  SET_TOPLEVEL)            \
  VISIT(3,  SET_CLASS_CLASS)      VISIT(4,  SET_LAMBDA_CLASS)        \
  VISIT(5,  SET_TUPLE_CLASS)      VISIT(6,  SET_CODE_CLASS)          \
  VISIT(7,  SET_STRING_CLASS)     VISIT(8,  SET_BUFFER_CLASS)        \
  VISIT(9,  SET_FALSE_CLASS)      VISIT(10, SET_FALSE)               \
  VISIT(11, SET_TRUE_CLASS)       VISIT(12, SET_TRUE)                \
  VISIT(13, SET_NULL_CLASS)       VISIT(14, SET_NULL)                \
  VISIT(15, SET_VOID_CLASS)       VISIT(16, SET_VOID)                \
  VISIT(17, SET_LITERAL_CLASS)    VISIT(18, SET_METHOD_CLASS)        \
  VISIT(19, SET_SMI_CLASS)                                           \
                                                                     \
  VISIT(30, NEW_DICTIONARY)       VISIT(31, NEW_STRING)              \
  VISIT(32, NEW_LAMBDA)           VISIT(33, NEW_CODE)                \
  VISIT(34, NEW_TUPLE)            VISIT(35, NEW_CLASS)               \
  VISIT(36, NEW_NUMBER)           VISIT(37, NEW_METHOD)              \
                                                                     \
  VISIT(40, LOAD_REGISTER)        VISIT(41, PENDING_REGISTER)        \
  VISIT(42, STORE_REGISTER)       VISIT(43, SCHEDULE_FIXUP)


// -----------------
// --- R o o t s ---
// -----------------

#define FOR_EACH_SIMPLE_ROOT(VISIT)                                  \
  VISIT(Class, string_class, STRING_CLASS, new_class(STRING_TYPE))   \
  VISIT(Class, tuple_class, TUPLE_CLASS, new_class(TUPLE_TYPE))      \
  VISIT(Class, void_class, VOID_CLASS, new_class(VOID_TYPE))         \
  VISIT(Void, vhoid, VOID, new_singleton(void_class()))              \
  VISIT(Class, null_class, NULL_CLASS, new_class(NULL_TYPE))         \
  VISIT(Null, nuhll, NULL, new_singleton(null_class()))              \
  VISIT(Class, true_class, TRUE_CLASS, new_class(TRUE_TYPE))         \
  VISIT(True, thrue, TRUE, new_singleton(true_class()))              \
  VISIT(Class, false_class, FALSE_CLASS, new_class(FALSE_TYPE))      \
  VISIT(False, fahlse, FALSE, new_singleton(false_class()))          \
  VISIT(Class, literal_class, LITERAL_CLASS, new_class(LITERAL_TYPE))\
  VISIT(Class, dictionary_class, DICTIONARY_CLASS, new_class(DICTIONARY_TYPE))\
  VISIT(Class, lambda_class, LAMBDA_CLASS, new_class(LAMBDA_TYPE))   \
  VISIT(Dictionary, toplevel, TOPLEVEL, new_dictionary())            \
  VISIT(Class, buffer_class, BUFFER_CLASS, new_class(BUFFER_TYPE))   \
  VISIT(Class, code_class, CODE_CLASS, new_class(CODE_TYPE))         \
  VISIT(Class, method_class, METHOD_CLASS, new_class(METHOD_TYPE))   \
  VISIT(Class, smi_class, SMI_CLASS, new_class(SMI_TYPE))

#define FOR_EACH_COMPLICATED_ROOT(VISIT)                             \
  VISIT(Class, class_class, CLASS_CLASS, NULL)

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
