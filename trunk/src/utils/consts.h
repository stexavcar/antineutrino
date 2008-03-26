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
  VISIT(1,  DICTIONARY,             Dictionary,            dictionary)             \
  VISIT(2,  METHOD,                 Method,                method)                 \
  VISIT(3,  PROTOCOL,               Protocol,              protocol)               \
  VISIT(4,  TASK,                   Task,                  task)                   \
  VISIT(73, SIGNATURE,              Signature,             signature)              \
  VISIT(74, FORWARDER_DESCRIPTOR,   ForwarderDescriptor,   forwarder_descriptor)   \
  VISIT(77, SELECTOR,               Selector,              selector)

#define FOR_EACH_OBJECT_TYPE(VISIT)                                                \
  VISIT(5,  LAYOUT,                 Layout,                layout)                 \
  VISIT(6,  STRING,                 String,                string)                 \
  VISIT(7,  TUPLE,                  Tuple,                 tuple)                  \
  VISIT(8,  VOID,                   Void,                  void)                   \
  VISIT(9,  NULL,                   Null,                  null)                   \
  VISIT(10, TRUE,                   True,                  true)                   \
  VISIT(11, FALSE,                  False,                 false)                  \
  VISIT(12, LAMBDA,                 Lambda,                lambda)                 \
  VISIT(13, BUFFER,                 Buffer,                buffer)                 \
  VISIT(14, CODE,                   Code,                  code)                   \
  VISIT(15, INSTANCE,               Instance,              instance)               \
  VISIT(16, STACK,                  Stack,                 stack)                  \
  VISIT(17, CONTEXT,                Context,               context)                \
  FOR_EACH_GENERATABLE_OBJECT_TYPE(VISIT)                                          \
  FOR_EACH_SYNTAX_TREE_TYPE(VISIT)

#define FOR_EACH_VIRTUAL_TYPE(VISIT)                                               \
  VISIT(20, OBJECT,                 Object,                object)                 \
  VISIT(21, VALUE,                  Value,                 0)                      \
  VISIT(22, ABSTRACT_BUFFER,        AbstractBuffer,        0)                      \
  VISIT(23, SINGLETON,              Singleton,             0)                      \
  VISIT(24, SYNTAX_TREE,            SyntaxTree,            0)                      \
  VISIT(25, IMMEDIATE,              Immediate,             0)                      \
  VISIT(26, BOOL,                   Bool,                  0)                      \
  VISIT(82, ROOT,                   Root,                  0)

#define FOR_EACH_SIGNAL_TYPE(VISIT)                                                \
  VISIT(30, ALLOCATION_FAILED,      AllocationFailed,      0)                      \
  VISIT(31, INTERNAL_ERROR,         InternalError,         0)                      \
  VISIT(32, NOTHING,                Nothing,               0)

#define FOR_EACH_VALUE_TYPE(VISIT)                                                 \
  VISIT(40, SMI,                    Smi,                   smi)                    \
  VISIT(41, SIGNAL,                 Signal,                0)                      \
  VISIT(42, FORWARD_POINTER,        ForwardPointer,        0)                      \
  VISIT(43, FORWARDER,              Forwarder,             0)                      \
  FOR_EACH_SIGNAL_TYPE(VISIT)                                                      \
  FOR_EACH_OBJECT_TYPE(VISIT)

#define FOR_EACH_DECLARED_TYPE(VISIT)                                              \
  FOR_EACH_VALUE_TYPE(VISIT)                                                       \
  FOR_EACH_VIRTUAL_TYPE(VISIT)

#define FOR_EACH_SYNTAX_TREE_TYPE(VISIT)                                           \
  VISIT(50, BUILTIN_CALL,           BuiltinCall,           builtin_call)           \
  VISIT(51, UNQUOTE_EXPRESSION,     UnquoteExpression,     unquote_expression)     \
  VISIT(52, QUOTE_TEMPLATE,         QuoteTemplate,         quote_template)         \
  FOR_EACH_GENERATABLE_SYNTAX_TREE_TYPE(VISIT)

#define FOR_EACH_GENERATABLE_SYNTAX_TREE_TYPE(VISIT)                               \
  VISIT(53, LITERAL_EXPRESSION,     LiteralExpression,     literal_expression)     \
  VISIT(54, INVOKE_EXPRESSION,      InvokeExpression,      invoke_expression)      \
  VISIT(55, PROTOCOL_EXPRESSION,    ProtocolExpression,    protocol_expression)    \
  VISIT(56, RETURN_EXPRESSION,      ReturnExpression,      return_expression)      \
  VISIT(57, METHOD_EXPRESSION,      MethodExpression,      method_expression)      \
  VISIT(58, SEQUENCE_EXPRESSION,    SequenceExpression,    sequence_expression)    \
  VISIT(59, TUPLE_EXPRESSION,       TupleExpression,       tuple_expression)       \
  VISIT(60, GLOBAL_EXPRESSION,      GlobalExpression,      global_expression)      \
  VISIT(61, CALL_EXPRESSION,        CallExpression,        call_expression)        \
  VISIT(62, SYMBOL,                 Symbol,                symbol)                 \
  VISIT(63, CONDITIONAL_EXPRESSION, ConditionalExpression, conditional_expression) \
  VISIT(64, QUOTE_EXPRESSION,       QuoteExpression,       quote_expression)       \
  VISIT(65, THIS_EXPRESSION,        ThisExpression,        this_expression)        \
  VISIT(66, LAMBDA_EXPRESSION,      LambdaExpression,      lambda_expression)      \
  VISIT(67, INTERPOLATE_EXPRESSION, InterpolateExpression, interpolate_expression) \
  VISIT(68, LOCAL_DEFINITION,       LocalDefinition,       local_definition)       \
  VISIT(69, RAISE_EXPRESSION,       RaiseExpression,       raise_expression)       \
  VISIT(70, ON_CLAUSE,              OnClause,              on_clause)              \
  VISIT(71, DO_ON_EXPRESSION,       DoOnExpression,        do_on_expression)       \
  VISIT(72, INSTANTIATE_EXPRESSION, InstantiateExpression, instantiate_expression) \
  VISIT(75, ARGUMENTS,              Arguments,             arguments)              \
  VISIT(76, EXTERNAL_CALL,          ExternalCall,          external_call)          \
  VISIT(78, TASK_EXPRESSION,        TaskExpression,        task_expression)        \
  VISIT(79, YIELD_EXPRESSION,       YieldExpression,       yield_expression)       \
  VISIT(80, ASSIGNMENT,             Assignment,            assignment)             \
  VISIT(81, PARAMETERS,             Parameters,            parameters)


#define FOR_EACH_GENERATABLE_TYPE(VISIT)                                           \
  FOR_EACH_GENERATABLE_OBJECT_TYPE(VISIT)                                          \
  FOR_EACH_GENERATABLE_SYNTAX_TREE_TYPE(VISIT)

// -------------------------------------
// --- I m a g e   C o n s t a n t s ---
// -------------------------------------

#define FOR_EACH_IMAGE_OBJECT_CONST(VISIT)                           \
  VISIT(0, Object,                LayoutOffset)                      \
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
  VISIT(5, Lambda,                ContextOffset)                     \
  VISIT(6, Lambda,                Size)                              \
  VISIT(1, Layout,                InstanceTypeOffset)                \
  VISIT(2, Layout,                MethodsOffset)                     \
  VISIT(3, Layout,                ProtocolOffset)                    \
  VISIT(4, Layout,                FieldCountOffset)                  \
  VISIT(5, Layout,                Size)                              \
  VISIT(1, Protocol,              MethodsOffset)                     \
  VISIT(2, Protocol,              NameOffset)                        \
  VISIT(3, Protocol,              SuperOffset)                       \
  VISIT(4, Protocol,              Size)                              \
  VISIT(1, Method,                SelectorOffset)                    \
  VISIT(2, Method,                SignatureOffset)                   \
  VISIT(3, Method,                LambdaOffset)                      \
  VISIT(4, Method,                Size)                              \
  VISIT(1, Signature,             ParametersOffset)                  \
  VISIT(2, Signature,             Size)                              \
  VISIT(1, Root,                  IndexOffset)                       \
  VISIT(2, Root,                  Size)                              \
  VISIT(1, LiteralExpression,     ValueOffset)                       \
  VISIT(2, LiteralExpression,     Size)                              \
  VISIT(1, InvokeExpression,      ReceiverOffset)                    \
  VISIT(2, InvokeExpression,      SelectorOffset)                    \
  VISIT(3, InvokeExpression,      ArgumentsOffset)                   \
  VISIT(4, InvokeExpression,      Size)                              \
  VISIT(1, InstantiateExpression, TermsOffset)                       \
  VISIT(2, InstantiateExpression, ReceiverOffset)                    \
  VISIT(3, InstantiateExpression, NameOffset)                        \
  VISIT(4, InstantiateExpression, ArgumentsOffset)                   \
  VISIT(5, InstantiateExpression, Size)                              \
  VISIT(1, ProtocolExpression,    NameOffset)                        \
  VISIT(2, ProtocolExpression,    MethodsOffset)                     \
  VISIT(3, ProtocolExpression,    SuperOffset)                       \
  VISIT(4, ProtocolExpression,    Size)                              \
  VISIT(1, ReturnExpression,      ValueOffset)                       \
  VISIT(2, ReturnExpression,      Size)                              \
  VISIT(1, MethodExpression,      SelectorOffset)                    \
  VISIT(2, MethodExpression,      LambdaOffset)                      \
  VISIT(3, MethodExpression,      IsStaticOffset)                    \
  VISIT(4, MethodExpression,      Size)                              \
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
  VISIT(1, LambdaExpression,      ParametersOffset)                  \
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
  VISIT(1, Assignment,            SymbolOffset)                      \
  VISIT(2, Assignment,            ValueOffset)                       \
  VISIT(3, Assignment,            Size)                              \
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
  VISIT(3, DoOnExpression,        Size)                              \
  VISIT(1, Context,               Size)                              \
  VISIT(1, ForwarderDescriptor,   TypeOffset)                        \
  VISIT(2, ForwarderDescriptor,   TargetOffset)                      \
  VISIT(3, ForwarderDescriptor,   Size)                              \
  VISIT(1, Arguments,             ArgumentsOffset)                   \
  VISIT(2, Arguments,             KeywordIndicesOffset)              \
  VISIT(3, Arguments,             Size)                              \
  VISIT(1, Parameters,            PositionCountOffset)               \
  VISIT(2, Parameters,            ParametersOffset)                  \
  VISIT(3, Parameters,            Size)                              \
  VISIT(1, ExternalCall,          ArgcOffset)                        \
  VISIT(2, ExternalCall,          NameOffset)                        \
  VISIT(3, ExternalCall,          Size)                              \
  VISIT(1, Selector,              NameOffset)                        \
  VISIT(2, Selector,              ArgcOffset)                        \
  VISIT(3, Selector,              KeywordsOffset)                    \
  VISIT(4, Selector,              IsAccessorOffset)                  \
  VISIT(5, Selector,              Size)                              \
  VISIT(1, TaskExpression,        LambdaOffset)                      \
  VISIT(2, TaskExpression,        Size)                              \
  VISIT(1, YieldExpression,       ValueOffset)                       \
  VISIT(2, YieldExpression,       Size)


// -----------------
// --- R o o t s ---
// -----------------

#define FOR_EACH_SIMPLE_ROOT_OBJECT(VISIT)                                                                                   \
  VISIT(0,  Void,       vhoid,                         VoidValue,             new_singleton(void_layout()))                  \
  VISIT(1,  Null,       nuhll,                         NullValue,             new_singleton(null_layout()))                  \
  VISIT(2,  True,       thrue,                         TrueValue,             new_singleton(true_layout()))                  \
  VISIT(3,  False,      fahlse,                        FalseValue,            new_singleton(false_layout()))                 \
  VISIT(4,  Dictionary, toplevel,                      Toplevel,              new_dictionary())                              \
  VISIT(5,  Tuple,      empty_tuple,                   EmptyTuple,            new_tuple(0))

#define FOR_EACH_COMPLICATED_ROOT_LAYOUT(VISIT)                                                                              \
  VISIT(6,  Layout,     layout_layout,                 Layout,                0)

#define FOR_EACH_SIMPLE_ROOT_LAYOUT(VISIT)                                                                                   \
  VISIT(7,  Layout,     protocol_layout,               Protocol,              allocate_empty_layout(tProtocol))              \
  VISIT(8,  Layout,     string_layout,                 String,                allocate_empty_layout(tString))                \
  VISIT(9,  Layout,     tuple_layout,                  Tuple,                 allocate_empty_layout(tTuple))                 \
  VISIT(10, Layout,     void_layout,                   Void,                  allocate_empty_layout(tVoid))                  \
  VISIT(11, Layout,     null_layout,                   Null,                  allocate_empty_layout(tNull))                  \
  VISIT(12, Layout,     true_layout,                   True,                  allocate_empty_layout(tTrue))                  \
  VISIT(13, Layout,     false_layout,                  False,                 allocate_empty_layout(tFalse))                 \
  VISIT(14, Layout,     literal_expression_layout,     LiteralExpression,     allocate_empty_layout(tLiteralExpression))     \
  VISIT(15, Layout,     dictionary_layout,             Dictionary,            allocate_empty_layout(tDictionary))            \
  VISIT(16, Layout,     lambda_layout,                 Lambda,                allocate_empty_layout(tLambda))                \
  VISIT(17, Layout,     buffer_layout,                 Buffer,                allocate_empty_layout(tBuffer))                \
  VISIT(18, Layout,     code_layout,                   Code,                  allocate_empty_layout(tCode))                  \
  VISIT(19, Layout,     method_layout,                 Method,                allocate_empty_layout(tMethod))                \
  VISIT(20, Layout,     smi_layout,                    SmallInteger,          allocate_empty_layout(tSmi))                   \
  VISIT(21, Layout,     invoke_expression_layout,      InvokeExpression,      allocate_empty_layout(tInvokeExpression))      \
  VISIT(22, Layout,     protocol_expression_layout,    ProtocolExpression,    allocate_empty_layout(tProtocolExpression))    \
  VISIT(23, Layout,     return_expression_layout,      ReturnExpression,      allocate_empty_layout(tReturnExpression))      \
  VISIT(24, Layout,     method_expression_layout,      MethodExpression,      allocate_empty_layout(tMethodExpression))      \
  VISIT(25, Layout,     sequence_expression_layout,    SequenceExpression,    allocate_empty_layout(tSequenceExpression))    \
  VISIT(26, Layout,     tuple_expression_layout,       TupleExpression,       allocate_empty_layout(tTupleExpression))       \
  VISIT(27, Layout,     global_expression_layout,      GlobalExpression,      allocate_empty_layout(tGlobalExpression))      \
  VISIT(28, Layout,     symbol_layout,                 Symbol,                allocate_empty_layout(tSymbol))                \
  VISIT(29, Layout,     call_expression_layout,        CallExpression,        allocate_empty_layout(tCallExpression))        \
  VISIT(30, Layout,     conditional_expression_layout, ConditionalExpression, allocate_empty_layout(tConditionalExpression)) \
  VISIT(31, Layout,     this_expression_layout,        ThisExpression,        allocate_empty_layout(tThisExpression))        \
  VISIT(32, Layout,     quote_expression_layout,       QuoteExpression,       allocate_empty_layout(tQuoteExpression))       \
  VISIT(33, Layout,     lambda_expression_layout,      LambdaExpression,      allocate_empty_layout(tLambdaExpression))      \
  VISIT(34, Layout,     builtin_call_layout,           BuiltinCall,           allocate_empty_layout(tBuiltinCall))           \
  VISIT(35, Layout,     interpolate_expression_layout, InterpolateExpression, allocate_empty_layout(tInterpolateExpression)) \
  VISIT(36, Layout,     local_definition_layout,       LocalDefinition,       allocate_empty_layout(tLocalDefinition))       \
  VISIT(37, Layout,     unquote_expression_layout,     UnquoteExpression,     allocate_empty_layout(tUnquoteExpression))     \
  VISIT(38, Layout,     quote_template_layout,         QuoteTemplate,         allocate_empty_layout(tQuoteTemplate))         \
  VISIT(39, Layout,     stack_layout,                  Stack,                 allocate_empty_layout(tStack))                 \
  VISIT(40, Layout,     task_layout,                   Task,                  allocate_empty_layout(tTask))                  \
  VISIT(41, Layout,     raise_expression_layout,       RaiseExpression,       allocate_empty_layout(tRaiseExpression))       \
  VISIT(42, Layout,     on_clause_layout,              OnClause,              allocate_empty_layout(tOnClause))              \
  VISIT(43, Layout,     do_on_expression_layout,       DoOnExpression,        allocate_empty_layout(tDoOnExpression))        \
  VISIT(44, Layout,     instantiate_expression_layout, InstantiateExpression, allocate_empty_layout(tInstantiateExpression)) \
  VISIT(45, Layout,     context_layout,                Context,               allocate_empty_layout(tContext))               \
  VISIT(46, Layout,     signature_layout,              Signature,             allocate_empty_layout(tSignature))             \
  VISIT(47, Layout,     forwarder_descriptor_layout,   ForwarderDescriptor,   allocate_empty_layout(tForwarderDescriptor))   \
  VISIT(48, Layout,     arguments_layout,              Arguments,             allocate_empty_layout(tArguments))             \
  VISIT(49, Layout,     external_call_layout,          ExternalCall,          allocate_empty_layout(tExternalCall))          \
  VISIT(50, Layout,     selector_layout,               Selector,              allocate_empty_layout(tSelector))              \
  VISIT(51, Layout,     task_expression_layout,        TaskExpression,        allocate_empty_layout(tTaskExpression))        \
  VISIT(52, Layout,     yield_expression_layout,       YieldExpression,       allocate_empty_layout(tYieldExpression))       \
  VISIT(53, Layout,     assignment_layout,             Assignment,            allocate_empty_layout(tAssignment))            \
  VISIT(54, Layout,     parameters_layout,             Parameters,            allocate_empty_layout(tParameters))


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
// --- B u i l t - i n   M e t h o d s ---
// ---------------------------------------

/**
 * This index defines the index, layout, name and source name for all
 * internal methods.  Whenever a method is added here an associated
 * implementation must be added in builtins.cc.
 */
#define FOR_EACH_BUILTIN_METHOD(VISIT)                               \
  VISIT(0,  string,              length,      "||")                  \
  VISIT(1,  string,              eq,          "=")                   \
  VISIT(2,  string,              plus,        "+")                   \
  VISIT(10, smi,                 plus,        "+")                   \
  VISIT(11, smi,                 minus,       "-")                   \
  VISIT(12, smi,                 times,       "×")                   \
  VISIT(13, smi,                 divide,      "÷")                   \
  VISIT(14, smi,                 abs,         "||")                  \
  VISIT(20, object,              eq,          "=")                   \
  VISIT(21, object,              to_string,   "to_string")           \
  VISIT(30, protocol_expression, evaluate,    "evaluate")            \
  VISIT(40, protocol,            new,         "new")                 \
  VISIT(41, tuple,               eq,          "=")                   \
  VISIT(42, lambda,              disassemble, "disassemble")         \
  VISIT(43, lambda_expression,   params,      "parameters")          \
  VISIT(44, lambda_expression,   body,        "body")


// ---------------------------------------
// --- S p e c i a l   B u i l t i n s ---
// ---------------------------------------

#define FOR_EACH_SPECIAL_BUILTIN_FUNCTION(VISIT)                     \
  VISIT(50, attach_task, "attach_task")


// -------------------------------------------
// --- B u i l t - i n   F u n c t i o n s ---
// -------------------------------------------

/**
 * This index defines the index, name and source name of all internal
 * functions.  Whenever a method is added here an associated
 * implementation must be added in builtins.cc.
 */
#define FOR_EACH_PLAIN_BUILTIN_FUNCTION(VISIT)                       \
  VISIT(101, raw_print,          "raw_print")                        \
  VISIT(102, compile_expression, "compile_expression")               \
  VISIT(103, lift,               "lift")                             \
  VISIT(104, make_forwarder,     "make_forwarder")                   \
  VISIT(105, set_target,         "set_target")

#define FOR_EACH_BUILTIN_FUNCTION(VISIT)                             \
  FOR_EACH_SPECIAL_BUILTIN_FUNCTION(VISIT)                           \
  FOR_EACH_PLAIN_BUILTIN_FUNCTION(VISIT)

#endif // _UTILS_CONSTS
