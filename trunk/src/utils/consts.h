#ifndef _UTILS_CONSTS
#define _UTILS_CONSTS

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

#define FOR_EACH_GENERATABLE_OBJECT_TYPE(VISIT)                                    \
  VISIT(4,  DICTIONARY,             Dictionary,            dictionary)             \
  VISIT(12, METHOD,                 Method,                method)                 \
  VISIT(13, PROTOCOL,               Protocol,              protocol)               \
  VISIT(16, TASK,                   Task,                  task)

#define FOR_EACH_OBJECT_TYPE(VISIT)                                                \
  VISIT(1,  LAYOUT,                 Layout,                0)                      \
  VISIT(2,  STRING,                 String,                0)                      \
  VISIT(3,  TUPLE,                  Tuple,                 0)                      \
  VISIT(5,  VOID,                   Void,                  0)                      \
  VISIT(6,  NULL,                   Null,                  0)                      \
  VISIT(7,  TRUE,                   True,                  0)                      \
  VISIT(8,  FALSE,                  False,                 0)                      \
  VISIT(9,  LAMBDA,                 Lambda,                0)                      \
  VISIT(10, BUFFER,                 Buffer,                0)                      \
  VISIT(11, CODE,                   Code,                  0)                      \
  VISIT(14, INSTANCE,               Instance,              0)                      \
  VISIT(15, STACK,                  Stack,                 0)                      \
  FOR_EACH_GENERATABLE_OBJECT_TYPE(VISIT)                                          \
  FOR_EACH_SYNTAX_TREE_TYPE(VISIT)

#define FOR_EACH_VIRTUAL_TYPE(VISIT)                                               \
  VISIT(20, OBJECT,                 Object,                0)                      \
  VISIT(21, VALUE,                  Value,                 0)                      \
  VISIT(22, ABSTRACT_BUFFER,        AbstractBuffer,        0)                      \
  VISIT(23, SINGLETON,              Singleton,             0)                      \
  VISIT(24, SYNTAX_TREE,            SyntaxTree,            0)

#define FOR_EACH_SIGNAL_TYPE(VISIT)                                                \
  VISIT(30, ALLOCATION_FAILED,      AllocationFailed,      0)                      \
  VISIT(31, INTERNAL_ERROR,         InternalError,         0)                      \
  VISIT(32, NOTHING,                Nothing,               0)

#define FOR_EACH_VALUE_TYPE(VISIT)                                                 \
  VISIT(40, SMI,                    Smi,                   0)                      \
  VISIT(41, SIGNAL,                 Signal,                0)                      \
  VISIT(42, FORWARD_POINTER,        ForwardPointer,        0)                      \
  FOR_EACH_SIGNAL_TYPE(VISIT)                                                      \
  FOR_EACH_OBJECT_TYPE(VISIT)

#define FOR_EACH_DECLARED_TYPE(VISIT)                                              \
  FOR_EACH_VALUE_TYPE(VISIT)                                                       \
  FOR_EACH_VIRTUAL_TYPE(VISIT)

#define FOR_EACH_SYNTAX_TREE_TYPE(VISIT)                                           \
  VISIT(64, BUILTIN_CALL,           BuiltinCall,           builtin_call)           \
  VISIT(67, UNQUOTE_EXPRESSION,     UnquoteExpression,     unquote_expression)     \
  VISIT(68, QUOTE_TEMPLATE,         QuoteTemplate,         quote_template)         \
  FOR_EACH_GENERATABLE_SYNTAX_TREE_TYPE(VISIT)

#define FOR_EACH_GENERATABLE_SYNTAX_TREE_TYPE(VISIT)                               \
  VISIT(50, LITERAL_EXPRESSION,     LiteralExpression,     literal_expression)     \
  VISIT(51, INVOKE_EXPRESSION,      InvokeExpression,      invoke_expression)      \
  VISIT(52, LAYOUT_EXPRESSION,      LayoutExpression,      layout_expression)      \
  VISIT(53, RETURN_EXPRESSION,      ReturnExpression,      return_expression)      \
  VISIT(54, METHOD_EXPRESSION,      MethodExpression,      method_expression)      \
  VISIT(55, SEQUENCE_EXPRESSION,    SequenceExpression,    sequence_expression)    \
  VISIT(56, TUPLE_EXPRESSION,       TupleExpression,       tuple_expression)       \
  VISIT(57, GLOBAL_EXPRESSION,      GlobalExpression,      global_expression)      \
  VISIT(58, CALL_EXPRESSION,        CallExpression,        call_expression)        \
  VISIT(59, SYMBOL,                 Symbol,                symbol)                 \
  VISIT(60, CONDITIONAL_EXPRESSION, ConditionalExpression, conditional_expression) \
  VISIT(61, QUOTE_EXPRESSION,       QuoteExpression,       quote_expression)       \
  VISIT(62, THIS_EXPRESSION,        ThisExpression,        this_expression)        \
  VISIT(63, LAMBDA_EXPRESSION,      LambdaExpression,      lambda_expression)      \
  VISIT(65, INTERPOLATE_EXPRESSION, InterpolateExpression, interpolate_expression) \
  VISIT(66, LOCAL_DEFINITION,       LocalDefinition,       local_definition)       \
  VISIT(69, RAISE_EXPRESSION,       RaiseExpression,       raise_expression)       \
  VISIT(70, ON_CLAUSE,              OnClause,              on_clause)              \
  VISIT(71, DO_ON_EXPRESSION,       DoOnExpression,        do_on_expression)       \
  VISIT(72, INSTANTIATE_EXPRESSION, InstantiateExpression, instantiate_expression)

#define FOR_EACH_GENERATABLE_TYPE(VISIT)                                           \
  FOR_EACH_GENERATABLE_OBJECT_TYPE(VISIT)                                          \
  FOR_EACH_GENERATABLE_SYNTAX_TREE_TYPE(VISIT)

// -------------------------------------
// --- I m a g e   C o n s t a n t s ---
// -------------------------------------

#define FOR_EACH_IMAGE_OBJECT_CONST(VISIT)                           \
  VISIT(0, Object,                TypeOffset)                        \
  VISIT(1, Object,                HeaderSize)                        \
  VISIT(1, Dictionary,            TableOffset)                       \
  VISIT(2, Dictionary,            Size)                              \
  VISIT(1, String,                LengthOffset)                      \
  VISIT(2, String,                HeaderSize)                        \
  VISIT(1, Code,                  LengthOffset)                      \
  VISIT(2, Code,                  HeaderSize)                        \
  VISIT(1, Tuple,                 LengthOffset)                      \
  VISIT(2, Tuple,                 HeaderSize)                        \
  VISIT(1, Lambda,                ArgcOffset)                        \
  VISIT(2, Lambda,                CodeOffset)                        \
  VISIT(3, Lambda,                LiteralsOffset)                    \
  VISIT(4, Lambda,                TreeOffset)                        \
  VISIT(5, Lambda,                Size)                              \
  VISIT(1, Layout,                InstanceTypeOffset)                \
  VISIT(2, Layout,                MethodsOffset)                     \
  VISIT(3, Layout,                SuperOffset)                       \
  VISIT(4, Layout,                NameOffset)                        \
  VISIT(5, Layout,                Size)                              \
  VISIT(1, Protocol,              MethodsOffset)                     \
  VISIT(2, Protocol,              NameOffset)                        \
  VISIT(3, Protocol,              Size)                              \
  VISIT(1, Method,                NameOffset)                        \
  VISIT(2, Method,                LambdaOffset)                      \
  VISIT(3, Method,                Size)                              \
  VISIT(1, Root,                  IndexOffset)                       \
  VISIT(2, Root,                  Size)                              \
  VISIT(1, LiteralExpression,     ValueOffset)                       \
  VISIT(2, LiteralExpression,     Size)                              \
  VISIT(1, InvokeExpression,      ReceiverOffset)                    \
  VISIT(2, InvokeExpression,      NameOffset)                        \
  VISIT(3, InvokeExpression,      ArgumentsOffset)                   \
  VISIT(4, InvokeExpression,      Size)                              \
  VISIT(1, InstantiateExpression, TermsOffset)                       \
  VISIT(2, InstantiateExpression, ReceiverOffset)                    \
  VISIT(3, InstantiateExpression, NameOffset)                        \
  VISIT(4, InstantiateExpression, ArgumentsOffset)                   \
  VISIT(5, InstantiateExpression, Size)                              \
  VISIT(1, LayoutExpression,      NameOffset)                        \
  VISIT(2, LayoutExpression,      MethodsOffset)                     \
  VISIT(3, LayoutExpression,      SuperOffset)                       \
  VISIT(4, LayoutExpression,      Size)                              \
  VISIT(1, ReturnExpression,      ValueOffset)                       \
  VISIT(2, ReturnExpression,      Size)                              \
  VISIT(1, MethodExpression,      NameOffset)                        \
  VISIT(2, MethodExpression,      LambdaOffset)                      \
  VISIT(3, MethodExpression,      Size)                              \
  VISIT(1, SequenceExpression,    ExpressionsOffset)                 \
  VISIT(2, SequenceExpression,    Size)                              \
  VISIT(1, TupleExpression,       ValuesOffset)                      \
  VISIT(2, TupleExpression,       Size)                              \
  VISIT(1, GlobalExpression,      NameOffset)                        \
  VISIT(2, GlobalExpression,      Size)                              \
  VISIT(1, CallExpression,        ReceiverOffset)                    \
  VISIT(2, CallExpression,        FunctionOffset)                    \
  VISIT(3, CallExpression,        ArgumentsOffset)                   \
  VISIT(4, CallExpression,        Size)                              \
  VISIT(1, Symbol,                NameOffset)                        \
  VISIT(2, Symbol,                Size)                              \
  VISIT(1, ConditionalExpression, ConditionOffset)                   \
  VISIT(2, ConditionalExpression, ThenPartOffset)                    \
  VISIT(3, ConditionalExpression, ElsePartOffset)                    \
  VISIT(4, ConditionalExpression, Size)                              \
  VISIT(1, ThisExpression,        Size)                              \
  VISIT(1, QuoteExpression,       ValueOffset)                       \
  VISIT(2, QuoteExpression,       UnquotesOffset)                    \
  VISIT(3, QuoteExpression,       Size)                              \
  VISIT(1, LambdaExpression,      ParamsOffset)                      \
  VISIT(2, LambdaExpression,      BodyOffset)                        \
  VISIT(3, LambdaExpression,      Size)                              \
  VISIT(1, BuiltinCall,           ArgcOffset)                        \
  VISIT(2, BuiltinCall,           IndexOffset)                       \
  VISIT(3, BuiltinCall,           Size)                              \
  VISIT(1, InterpolateExpression, TermsOffset)                       \
  VISIT(2, InterpolateExpression, Size)                              \
  VISIT(1, LocalDefinition,       SymbolOffset)                      \
  VISIT(2, LocalDefinition,       ValueOffset)                       \
  VISIT(3, LocalDefinition,       BodyOffset)                        \
  VISIT(4, LocalDefinition,       Size)                              \
  VISIT(1, UnquoteExpression,     IndexOffset)                       \
  VISIT(2, UnquoteExpression,     Size)                              \
  VISIT(1, QuoteTemplate,         ValueOffset)                       \
  VISIT(2, QuoteTemplate,         UnquotesOffset)                    \
  VISIT(3, QuoteTemplate,         Size)                              \
  VISIT(1, Task,                  StackOffset)                       \
  VISIT(2, Task,                  Size)                              \
  VISIT(1, RaiseExpression,       NameOffset)                        \
  VISIT(2, RaiseExpression,       ArgumentsOffset)                   \
  VISIT(3, RaiseExpression,       Size)                              \
  VISIT(1, OnClause,              NameOffset)                        \
  VISIT(2, OnClause,              LambdaOffset)                      \
  VISIT(3, OnClause,              Size)                              \
  VISIT(1, DoOnExpression,        ValueOffset)                       \
  VISIT(2, DoOnExpression,        ClausesOffset)                     \
  VISIT(3, DoOnExpression,        Size)


// -----------------
// --- R o o t s ---
// -----------------

#define FOR_EACH_SIMPLE_ROOT_OBJECT(VISIT)                                                                                        \
  VISIT(0,  Void,       vhoid,                         VoidValue,             new_singleton(void_layout()))                       \
  VISIT(1,  Null,       nuhll,                         NullValue,             new_singleton(null_layout()))                       \
  VISIT(2,  True,       thrue,                         TrueValue,             new_singleton(true_layout()))                       \
  VISIT(3,  False,      fahlse,                        FalseValue,            new_singleton(false_layout()))                      \
  VISIT(4,  Dictionary, toplevel,                      Toplevel,              new_dictionary())                                   \
  VISIT(5,  Tuple,      empty_tuple,                   EmptyTuple,            new_tuple(0))

#define FOR_EACH_COMPLICATED_ROOT_LAYOUT(VISIT)                                                                                   \
  VISIT(6,  Layout,     layout_layout,                 Layout,                0)

#define FOR_EACH_SIMPLE_ROOT_LAYOUT(VISIT)                                                                                        \
  VISIT(44, Layout,     protocol_layout,               Protocol,              allocate_empty_layout(PROTOCOL_TYPE))               \
  VISIT(7,  Layout,     string_layout,                 String,                allocate_empty_layout(STRING_TYPE))                 \
  VISIT(8,  Layout,     tuple_layout,                  Tuple,                 allocate_empty_layout(TUPLE_TYPE))                  \
  VISIT(9,  Layout,     void_layout,                   Void,                  allocate_empty_layout(VOID_TYPE))                   \
  VISIT(10, Layout,     null_layout,                   Null,                  allocate_empty_layout(NULL_TYPE))                   \
  VISIT(11, Layout,     true_layout,                   True,                  allocate_empty_layout(TRUE_TYPE))                   \
  VISIT(12, Layout,     false_layout,                  False,                 allocate_empty_layout(FALSE_TYPE))                  \
  VISIT(13, Layout,     literal_expression_layout,     LiteralExpression,     allocate_empty_layout(LITERAL_EXPRESSION_TYPE))     \
  VISIT(14, Layout,     dictionary_layout,             Dictionary,            allocate_empty_layout(DICTIONARY_TYPE))             \
  VISIT(15, Layout,     lambda_layout,                 Lambda,                allocate_empty_layout(LAMBDA_TYPE))                 \
  VISIT(16, Layout,     buffer_layout,                 Buffer,                allocate_empty_layout(BUFFER_TYPE))                 \
  VISIT(17, Layout,     code_layout,                   Code,                  allocate_empty_layout(CODE_TYPE))                   \
  VISIT(18, Layout,     method_layout,                 Method,                allocate_empty_layout(METHOD_TYPE))                 \
  VISIT(19, Layout,     smi_layout,                    SmallInteger,          allocate_empty_layout(SMI_TYPE))                    \
  VISIT(20, Layout,     invoke_expression_layout,      InvokeExpression,      allocate_empty_layout(INVOKE_EXPRESSION_TYPE))      \
  VISIT(21, Layout,     layout_expression_layout,      LayoutExpression,      allocate_empty_layout(LAYOUT_EXPRESSION_TYPE))      \
  VISIT(22, Layout,     return_expression_layout,      ReturnExpression,      allocate_empty_layout(RETURN_EXPRESSION_TYPE))      \
  VISIT(23, Layout,     method_expression_layout,      MethodExpression,      allocate_empty_layout(METHOD_EXPRESSION_TYPE))      \
  VISIT(24, Layout,     sequence_expression_layout,    SequenceExpression,    allocate_empty_layout(SEQUENCE_EXPRESSION_TYPE))    \
  VISIT(25, Layout,     tuple_expression_layout,       TupleExpression,       allocate_empty_layout(TUPLE_EXPRESSION_TYPE))       \
  VISIT(26, Layout,     global_expression_layout,      GlobalExpression,      allocate_empty_layout(GLOBAL_EXPRESSION_TYPE))      \
  VISIT(27, Layout,     symbol_layout,                 Symbol,                allocate_empty_layout(SYMBOL_TYPE))                 \
  VISIT(28, Layout,     call_expression_layout,        CallExpression,        allocate_empty_layout(CALL_EXPRESSION_TYPE))        \
  VISIT(29, Layout,     conditional_expression_layout, ConditionalExpression, allocate_empty_layout(CONDITIONAL_EXPRESSION_TYPE)) \
  VISIT(30, Layout,     this_expression_layout,        ThisExpression,        allocate_empty_layout(THIS_EXPRESSION_TYPE))        \
  VISIT(31, Layout,     quote_expression_layout,       QuoteExpression,       allocate_empty_layout(QUOTE_EXPRESSION_TYPE))       \
  VISIT(32, Layout,     lambda_expression_layout,      LambdaExpression,      allocate_empty_layout(LAMBDA_EXPRESSION_TYPE))      \
  VISIT(33, Layout,     builtin_call_layout,           BuiltinCall,           allocate_empty_layout(BUILTIN_CALL_TYPE))           \
  VISIT(34, Layout,     interpolate_expression_layout, InterpolateExpression, allocate_empty_layout(INTERPOLATE_EXPRESSION_TYPE)) \
  VISIT(35, Layout,     local_definition_layout,       LocalDefinition,       allocate_empty_layout(LOCAL_DEFINITION_TYPE))       \
  VISIT(36, Layout,     unquote_expression_layout,     UnquoteExpression,     allocate_empty_layout(UNQUOTE_EXPRESSION_TYPE))     \
  VISIT(37, Layout,     quote_template_layout,         QuoteTemplate,         allocate_empty_layout(QUOTE_TEMPLATE_TYPE))         \
  VISIT(38, Layout,     stack_layout,                  Stack,                 allocate_empty_layout(STACK_TYPE))                  \
  VISIT(39, Layout,     task_layout,                   Task,                  allocate_empty_layout(TASK_TYPE))                   \
  VISIT(40, Layout,     raise_expression_layout,       RaiseExpression,       allocate_empty_layout(RAISE_EXPRESSION_TYPE))       \
  VISIT(41, Layout,     on_clause_layout,              OnClause,              allocate_empty_layout(ON_CLAUSE_TYPE))              \
  VISIT(42, Layout,     do_on_expression_layout,       DoOnExpression,        allocate_empty_layout(DO_ON_EXPRESSION_TYPE))       \
  VISIT(43, Layout,     instantiate_expression_layout, InstantiateExpression, allocate_empty_layout(INSTANTIATE_EXPRESSION_TYPE))

#define FOR_EACH_ROOT(VISIT)                                         \
  FOR_EACH_COMPLICATED_ROOT_LAYOUT(VISIT)                            \
  FOR_EACH_SIMPLE_ROOT(VISIT)

#define FOR_EACH_SIMPLE_ROOT(VISIT)                                  \
  FOR_EACH_SIMPLE_ROOT_LAYOUT(VISIT)                                 \
  FOR_EACH_SIMPLE_ROOT_OBJECT(VISIT)

#define FOR_EACH_ROOT_LAYOUT(VISIT)                                  \
  FOR_EACH_COMPLICATED_ROOT_LAYOUT(VISIT)                            \
  FOR_EACH_SIMPLE_ROOT_LAYOUT(VISIT)

// ---------------------------------------
// --- B u i l t - i n   C l a s s e s ---
// ---------------------------------------

/**
 * This index gives a mapping from built-in source layout names to root
 * names and associated enumeration values.  Any layout that is a root
 * or has built in methods should be mentioned in this list.
 */
#define FOR_EACH_BUILTIN_LAYOUT(VISIT)                                          \
  VISIT(SmallInteger,          smi,                    SMI)                    \
  VISIT(String,                string,                 STRING)                 \
  VISIT(True,                  true,                   TRUE)                   \
  VISIT(False,                 false,                  FALSE)                  \
  VISIT(Void,                  void,                   VOID)                   \
  VISIT(Null,                  null,                   NULL)                   \
  VISIT(Object,                object,                 OBJECT)                 \
  VISIT(Expression,            expression,             EXPRESSION)             \
  VISIT(Lambda,                lambda,                 LAMBDA)                 \
  VISIT(Tuple,                 tuple,                  TUPLE)                  \
  VISIT(Layout,                layout,                 LAYOUT)                 \
  VISIT(Dictionary,            dictionary,             DICTIONARY)             \
  VISIT(Buffer,                buffer,                 BUFFER)                 \
  VISIT(Code,                  code,                   CODE)                   \
  VISIT(Method,                method,                 METHOD)                 \
  VISIT(LiteralExpression,     literal_expression,     LITERAL_EXPRESSION)     \
  VISIT(InvokeExpression,      invoke_expression,      INVOKE_EXPRESSION)      \
  VISIT(LayoutExpression,      layout_expression,      LAYOUT_EXPRESSION)      \
  VISIT(ThisExpression,        this_expression,        THIS_EXPRESSION)        \
  VISIT(LocalDefinition,       local_definition,       LOCAL_DEFINITION)       \
  VISIT(LambdaExpression,      lambda_expression,      LAMBDA_EXPRESSION)      \
  VISIT(ReturnExpression,      return_expression,      RETURN_EXPRESSION)      \
  VISIT(CallExpression,        call_expression,        CALL_EXPRESSION)        \
  VISIT(MethodExpression,      method_expression,      METHOD_EXPRESSION)      \
  VISIT(SequenceExpression,    sequence_expression,    SEQUENCE_EXPRESSION)    \
  VISIT(TupleExpression,       tuple_expression,       TUPLE_EXPRESSION)       \
  VISIT(GlobalExpression,      global_expression,      GLOBAL_EXPRESSION)      \
  VISIT(Symbol,                symbol,                 SYMBOL)                 \
  VISIT(ConditionalExpression, conditional_expression, CONDITIONAL_EXPRESSION) \
  VISIT(QuoteExpression,       quote_expression,       QUOTE_EXPRESSION)       \
  VISIT(BuiltinCall,           builtin_call,           BUILTIN_CALL)           \
  VISIT(InterpolateExpression, interpolate_expression, INTERPOLATE_EXPRESSION) \
  VISIT(LocalDefinition,       local_definition,       LOCAL_DEFINITION)       \
  VISIT(QuoteTemplate,         quote_template,         QUOTE_TEMPLATE)


// ---------------------------------------
// --- B u i l t - i n   M e t h o d s ---
// ---------------------------------------

/**
 * This index defines the index, layout, name and source name for all
 * internal methods.  Whenever a method is added here an associated
 * implementation must be added in builtins.cc.
 */
#define FOR_EACH_BUILTIN_METHOD(VISIT)                               \
  VISIT(0,  string,            length,      "||")                    \
  VISIT(1,  string,            eq,          "=")                     \
  VISIT(2,  string,            plus,        "+")                     \
  VISIT(10, smi,               plus,        "+")                     \
  VISIT(11, smi,               minus,       "-")                     \
  VISIT(12, smi,               times,       "*")                     \
  VISIT(13, smi,               divide,      "/")                     \
  VISIT(14, smi,               abs,         "||")                    \
  VISIT(20, object,            eq,          "=")                     \
  VISIT(21, object,            to_string,   "to_string")             \
  VISIT(30, layout_expression, evaluate,    "evaluate")              \
  VISIT(40, layout,            new,         "new")                   \
  VISIT(41, tuple,             eq,          "=")                     \
  VISIT(42, lambda,            disassemble, "disassemble")           \
  VISIT(43, lambda_expression, params,      "parameters")            \
  VISIT(44, lambda_expression, body,        "body")


// -------------------------------------------
// --- B u i l t - i n   F u n c t i o n s ---
// -------------------------------------------

/**
 * This index defines the index, name and source name of all internal
 * functions.  Whenever a method is added here an associated
 * implementation must be added in builtins.cc.
 */
#define FOR_EACH_BUILTIN_FUNCTION(VISIT)                             \
  VISIT(101, raw_print,          "raw_print")                        \
  VISIT(102, compile_expression, "compile_expression")               \
  VISIT(103, lift,               "lift")


#endif // _UTILS_CONSTS
