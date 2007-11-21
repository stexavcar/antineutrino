#ifndef _HEAP_CONSTS
#define _HEAP_CONSTS

/**
 * This file contains a number of index tables mapping names and
 * properties to integer indices.  This file is written in a
 * particular style, with all values being defined through FOR_EACH
 * macros.  This is important because this file will be parsed by
 * the python compiler so that we only need to define the constants
 * in one place.
 */

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
  VISIT(14, INSTANCE, Instance)                                      \
  FOR_EACH_SYNTAX_TREE_TYPE(VISIT)

#define FOR_EACH_VIRTUAL_TYPE(VISIT)                                 \
  VISIT(20, OBJECT, Object)                                          \
  VISIT(21, VALUE, Value)                                            \
  VISIT(22, ABSTRACT_BUFFER, AbstractBuffer)                         \
  VISIT(23, SINGLETON, Singleton)                                    \
  VISIT(24, SYNTAX_TREE, SyntaxTree)

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
  VISIT(50, LITERAL_EXPRESSION, LiteralExpression)                   \
  VISIT(51, INVOKE_EXPRESSION, InvokeExpression)                     \
  VISIT(52, CLASS_EXPRESSION, ClassExpression)                       \
  VISIT(53, RETURN_EXPRESSION, ReturnExpression)                     \
  VISIT(54, METHOD_EXPRESSION, MethodExpression)                     \
  VISIT(55, SEQUENCE_EXPRESSION, SequenceExpression)                 \
  VISIT(56, TUPLE_EXPRESSION, TupleExpression)                       \
  VISIT(57, GLOBAL_EXPRESSION, GlobalExpression)                     \
  VISIT(58, CALL_EXPRESSION, CallExpression)                         \
  VISIT(59, SYMBOL, Symbol)


// ---------------------
// --- O p c o d e s ---
// ---------------------

/**
 * This index defines all opcodes, their index and the number of
 * arguments they expect.
 */
#define FOR_EACH_OPCODE(VISIT)                                       \
  VISIT(0,  PUSH,  1)   VISIT(1,  RETURN, 0) VISIT(2,  GLOBAL,   1)  \
  VISIT(3,  CALL,  1)   VISIT(4,  SLAP,   1) VISIT(5,  ARGUMENT, 1)  \
  VISIT(6,  VOID,  0)   VISIT(7,  NULL,  0)  VISIT(8,  TRUE,     0)  \
  VISIT(9,  FALSE, 0)   VISIT(10, POP,    1) VISIT(11, IF_TRUE,  1)  \
  VISIT(12, GOTO,  1)   VISIT(13, INVOKE, 2) VISIT(14, INTERNAL, 2)  \
  VISIT(15, TUPLE, 1)


// -------------------------------------
// --- I m a g e   C o n s t a n t s ---
// -------------------------------------

#define FOR_EACH_IMAGE_OBJECT_CONST(VISIT)                           \
  VISIT(0, Object, TypeOffset)        VISIT(1, Object, HeaderSize)   \
  VISIT(1, Dictionary, TableOffset)   VISIT(2, Dictionary, Size)     \
  VISIT(1, String, LengthOffset)      VISIT(2, String, HeaderSize)   \
  VISIT(1, Code, LengthOffset)        VISIT(2, Code, HeaderSize)     \
  VISIT(1, Tuple, LengthOffset)       VISIT(2, Tuple, HeaderSize)    \
  VISIT(1, Lambda, ArgcOffset)        VISIT(2, Lambda, CodeOffset)   \
  VISIT(3, Lambda, LiteralsOffset)    VISIT(4, Lambda, Size)         \
  VISIT(1, Class, InstanceTypeOffset) VISIT(2, Class, MethodsOffset) \
  VISIT(3, Class, SuperOffset)        VISIT(4, Class, NameOffset)    \
  VISIT(5, Class, Size)                                              \
  VISIT(1, Method, NameOffset)        VISIT(2, Method, LambdaOffset) \
  VISIT(3, Method, Size)                                             \
  VISIT(1, Root, IndexOffset)         VISIT(2, Root, Size)           \
  VISIT(1, LiteralExpression, ValueOffset)                           \
  VISIT(2, LiteralExpression, Size)                                  \
  VISIT(1, InvokeExpression, ReceiverOffset)                         \
  VISIT(2, InvokeExpression, NameOffset)                             \
  VISIT(3, InvokeExpression, ArgumentsOffset)                        \
  VISIT(4, InvokeExpression, Size)                                   \
  VISIT(1, ClassExpression, NameOffset)                              \
  VISIT(2, ClassExpression, MethodsOffset)                           \
  VISIT(3, ClassExpression, SuperOffset)                             \
  VISIT(4, ClassExpression, Size)                                    \
  VISIT(1, ReturnExpression, ValueOffset)                            \
  VISIT(2, ReturnExpression, Size)                                   \
  VISIT(1, MethodExpression, NameOffset)                             \
  VISIT(2, MethodExpression, BodyOffset)                             \
  VISIT(3, MethodExpression, Size)                                   \
  VISIT(1, SequenceExpression, ExpressionsOffset)                    \
  VISIT(2, SequenceExpression, Size)                                 \
  VISIT(1, TupleExpression, ValuesOffset)                            \
  VISIT(2, TupleExpression, Size)                                    \
  VISIT(1, GlobalExpression, NameOffset)                             \
  VISIT(2, GlobalExpression, Size)                                   \
  VISIT(1, CallExpression, ReceiverOffset)                           \
  VISIT(2, CallExpression, FunctionOffset)                           \
  VISIT(3, CallExpression, ArgumentsOffset)                          \
  VISIT(4, CallExpression, Size)                                     \
  VISIT(1, Symbol, NameOffset)                                       \
  VISIT(2, Symbol, Size)                                             \


// -----------------
// --- R o o t s ---
// -----------------

#define FOR_EACH_SIMPLE_ROOT_OBJECT(VISIT)                           \
  VISIT(0,  Void, vhoid, VOID, new_singleton(void_class()))          \
  VISIT(1,  Null, nuhll, NULL, new_singleton(null_class()))          \
  VISIT(2,  True, thrue, TRUE, new_singleton(true_class()))          \
  VISIT(3,  False, fahlse, FALSE, new_singleton(false_class()))      \
  VISIT(4,  Dictionary, toplevel, TOPLEVEL, new_dictionary())       \
  VISIT(5,  Tuple, empty_tuple, EMPTY_TUPLE, new_tuple(0))

#define FOR_EACH_COMPLICATED_ROOT_CLASS(VISIT)                       \
  VISIT(6, Class, class_class, CLASS_CLASS, NULL)

#define FOR_EACH_SIMPLE_ROOT_CLASS(VISIT)                            \
  VISIT(7,  Class, string_class, STRING_CLASS, new_empty_class(STRING_TYPE))\
  VISIT(8,  Class, tuple_class, TUPLE_CLASS, new_empty_class(TUPLE_TYPE))\
  VISIT(9,  Class, void_class, VOID_CLASS, new_empty_class(VOID_TYPE))\
  VISIT(10, Class, null_class, NULL_CLASS, new_empty_class(NULL_TYPE))\
  VISIT(11, Class, true_class, TRUE_CLASS, new_empty_class(TRUE_TYPE))\
  VISIT(12, Class, false_class, FALSE_CLASS, new_empty_class(FALSE_TYPE))\
  VISIT(13, Class, literal_expression_class, LITERAL_EXPRESSION_CLASS, new_empty_class(LITERAL_EXPRESSION_TYPE))\
  VISIT(14, Class, dictionary_class, DICTIONARY_CLASS, new_empty_class(DICTIONARY_TYPE))\
  VISIT(15, Class, lambda_class, LAMBDA_CLASS, new_empty_class(LAMBDA_TYPE))\
  VISIT(16, Class, buffer_class, BUFFER_CLASS, new_empty_class(BUFFER_TYPE))\
  VISIT(17, Class, code_class, CODE_CLASS, new_empty_class(CODE_TYPE))\
  VISIT(18, Class, method_class, METHOD_CLASS, new_empty_class(METHOD_TYPE))\
  VISIT(19, Class, smi_class, SMI_CLASS, new_empty_class(SMI_TYPE))  \
  VISIT(20, Class, invoke_expression_class, INVOKE_EXPRESSION_CLASS, new_empty_class(INVOKE_EXPRESSION_TYPE))\
  VISIT(21, Class, class_expression_class, CLASS_EXPRESSION_CLASS, new_empty_class(CLASS_EXPRESSION_TYPE))\
  VISIT(22, Class, return_expression_class, RETURN_EXPRESSION_CLASS, new_empty_class(RETURN_EXPRESSION_TYPE))\
  VISIT(23, Class, method_expression_class, METHOD_EXPRESSION_CLASS, new_empty_class(METHOD_EXPRESSION_TYPE))\
  VISIT(24, Class, sequence_expression_class, SEQUENCE_EXPRESSION_CLASS, new_empty_class(SEQUENCE_EXPRESSION_TYPE))\
  VISIT(25, Class, tuple_expression_class, TUPLE_EXPRESSION_CLASS, new_empty_class(TUPLE_EXPRESSION_TYPE))\
  VISIT(26, Class, global_expression_class, GLOBAL_EXPRESSION_CLASS, new_empty_class(GLOBAL_EXPRESSION_TYPE))\
  VISIT(27, Class, symbol_class, SYMBOL_CLASS, new_empty_class(SYMBOL_TYPE))\
  VISIT(28, Class, call_expression_class, CALL_EXPRESSION_CLASS, new_empty_class(CALL_EXPRESSION_TYPE))

#define FOR_EACH_ROOT(VISIT)                                         \
  FOR_EACH_COMPLICATED_ROOT_CLASS(VISIT)                             \
  FOR_EACH_SIMPLE_ROOT(VISIT)

#define FOR_EACH_SIMPLE_ROOT(VISIT)                                  \
  FOR_EACH_SIMPLE_ROOT_CLASS(VISIT)                                  \
  FOR_EACH_SIMPLE_ROOT_OBJECT(VISIT)

#define FOR_EACH_ROOT_CLASS(VISIT)                                   \
  FOR_EACH_COMPLICATED_ROOT_CLASS(VISIT)                             \
  FOR_EACH_SIMPLE_ROOT_CLASS(VISIT)

// ---------------------------------------
// --- B u i l t - i n   C l a s s e s ---
// ---------------------------------------

/**
 * This index gives a mapping from built-in source class names to root
 * names and associated enumeration values.  Any class that is a root
 * or has built in methods should be mentioned in this list.
 */
#define FOR_EACH_BUILTIN_CLASS(VISIT)                                \
  VISIT(SmallInteger, smi,  SMI)   VISIT(String, string, STRING)     \
  VISIT(True, true, TRUE)          VISIT(False,  false,  FALSE)      \
  VISIT(Void, void, VOID)          VISIT(Null,   null,   NULL)       \
  VISIT(Object, object, OBJECT)    VISIT(Lambda, lambda, LAMBDA)     \
  VISIT(Tuple, tuple, TUPLE)       VISIT(Class, class, CLASS)        \
  VISIT(LiteralExpression, literal_expression, LITERAL_EXPRESSION)   \
  VISIT(InvokeExpression, invoke_expression, INVOKE_EXPRESSION)      \
  VISIT(ClassExpression, class_expression, CLASS_EXPRESSION)


// ---------------------------------------
// --- B u i l t - i n   M e t h o d s ---
// ---------------------------------------

/**
 * This index defines the index, class, name and source name for all
 * internal methods.  Whenever a method is added here an associated
 * implementation must be added in builtins.cc.
 */
#define FOR_EACH_BUILTIN_METHOD(VISIT)                               \
  VISIT(0, string, length, "||")                                     \
  VISIT(1, string, eq, "=")                                          \
  VISIT(2, string, plus, "+")                                        \
  VISIT(10, smi, plus, "+")                                          \
  VISIT(11, smi, minus, "-")                                         \
  VISIT(12, smi, times, "*")                                         \
  VISIT(13, smi, divide, "/")                                        \
  VISIT(14, smi, abs, "||")                                          \
  VISIT(20, object, eq, "=")                                         \
  VISIT(21, object, to_string, "to_string")                          \
  VISIT(30, class_expression, evaluate, "evaluate")                  \
  VISIT(40, class, new, "new")                                       \
  VISIT(50, tuple, eq, "=")


// -------------------------------------------
// --- B u i l t - i n   F u n c t i o n s ---
// -------------------------------------------

/**
 * This index defines the index, name and source name of all internal
 * functions.  Whenever a method is added here an associated
 * implementation must be added in builtins.cc.
 */
#define FOR_EACH_BUILTIN_FUNCTION(VISIT)                             \
  VISIT(100, fail, "fail") VISIT(101, raw_print, "raw_print")


#endif // _HEAP_CONSTS
