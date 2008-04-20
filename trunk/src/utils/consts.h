#ifndef _UTILS_CONSTS
#define _UTILS_CONSTS

/**
 * This file contains a number of index tables mapping names and
 * properties to integer indices.  This file is written in a
 * particular style, with all values being defined through i...s
 * macros.  This is important because this file will be parsed by
 * the python compiler so that we only need to define the constants
 * in one place.
 */

// ---------------------------
// --- C l a s s   T a g s ---
// ---------------------------


#define eGeneratableObjectTypes(VISIT)                     \
  VISIT(1,  Dictionary,            dictionary)             \
  VISIT(2,  Method,                method)                 \
  VISIT(3,  Protocol,              protocol)               \
  VISIT(4,  Task,                  task)                   \
  VISIT(5,  Signature,             signature)              \
  VISIT(6,  Selector,              selector)


#define eObjectTypes(VISIT)                                \
  VISIT(7,  Layout,                layout)                 \
  VISIT(8,  String,                string)                 \
  VISIT(9,  Tuple,                 tuple)                  \
  VISIT(10, Array,                 array)                  \
  VISIT(11, Void,                  void)                   \
  VISIT(12, Null,                  null)                   \
  VISIT(13, True,                  true)                   \
  VISIT(14, False,                 false)                  \
  VISIT(15, Lambda,                lambda)                 \
  VISIT(16, Buffer,                buffer)                 \
  VISIT(17, Code,                  code)                   \
  VISIT(18, Instance,              instance)               \
  VISIT(19, Stack,                 stack)                  \
  VISIT(20, Context,               context)                \
  VISIT(21, ForwarderDescriptor,   forwarder_descriptor)   \
  VISIT(22, Channel,               channel)                \
  eGeneratableObjectTypes(VISIT)                           \
  eSyntaxTreeTypes(VISIT)


#define eVirtualTypes(VISIT)                               \
  VISIT(23, Object,                object)                 \
  VISIT(24, Value,                 0)                      \
  VISIT(25, AbstractBuffer,        0)                      \
  VISIT(26, AbstractTuple,         0)                      \
  VISIT(27, Singleton,             0)                      \
  VISIT(28, SyntaxTree,            0)                      \
  VISIT(29, Immediate,             0)                      \
  VISIT(30, Bool,                  0)                      \
  VISIT(31, Root,                  0)


#define eSignalTypes(VISIT)                                \
  VISIT(32, AllocationFailed,      0)                      \
  VISIT(33, InternalError,         0)                      \
  VISIT(34, Nothing,               0)


#define eValueTypes(VISIT)                                 \
  VISIT(35, Smi,                   smi)                    \
  VISIT(36, Signal,                0)                      \
  VISIT(37, ForwardPointer,        0)                      \
  VISIT(38, Forwarder,             0)                      \
  eSignalTypes(VISIT)                                      \
  eObjectTypes(VISIT)


#define eDeclaredTypes(VISIT)                              \
  eValueTypes(VISIT)                                       \
  eVirtualTypes(VISIT)


#define eSyntaxTreeTypes(VISIT)                            \
  VISIT(39, BuiltinCall,           builtin_call)           \
  VISIT(40, UnquoteExpression,     unquote_expression)     \
  VISIT(41, QuoteTemplate,         quote_template)         \
  VISIT(51, Symbol,                symbol)                 \
  VISIT(62, Arguments,             arguments)              \
  VISIT(61, InstantiateExpression, instantiate_expression) \
  VISIT(56, InterpolateExpression, interpolate_expression) \
  VISIT(53, QuoteExpression,       quote_expression)       \
  eSimpleSyntaxTreeTypes(VISIT)


#define eSimpleSyntaxTreeTypes(VISIT)                      \
  VISIT(42, LiteralExpression,     literal_expression)     \
  VISIT(43, InvokeExpression,      invoke_expression)      \
  VISIT(44, ProtocolExpression,    protocol_expression)    \
  VISIT(45, ReturnExpression,      return_expression)      \
  VISIT(46, MethodExpression,      method_expression)      \
  VISIT(47, SequenceExpression,    sequence_expression)    \
  VISIT(48, TupleExpression,       tuple_expression)       \
  VISIT(49, GlobalExpression,      global_expression)      \
  VISIT(50, CallExpression,        call_expression)        \
  VISIT(52, ConditionalExpression, conditional_expression) \
  VISIT(54, ThisExpression,        this_expression)        \
  VISIT(55, LambdaExpression,      lambda_expression)      \
  VISIT(57, LocalDefinition,       local_definition)       \
  VISIT(58, RaiseExpression,       raise_expression)       \
  VISIT(59, OnClause,              on_clause)              \
  VISIT(60, DoOnExpression,        do_on_expression)       \
  VISIT(63, TaskExpression,        task_expression)        \
  VISIT(64, YieldExpression,       yield_expression)       \
  VISIT(65, Assignment,            assignment)             \
  VISIT(66, Parameters,            parameters)             \
  VISIT(67, WhileExpression,       while_expression)       \
  VISIT(68, SuperExpression,       super_expression)


#define eGeneratableTypes(VISIT)                           \
  eGeneratableObjectTypes(VISIT)                           \
  eSimpleSyntaxTreeTypes(VISIT)


// -----------------------------------------
// --- B o i l e r p l a t e   l i s t s ---
// -----------------------------------------


/**
 * For each type for which a boilerplate allocator can be generated
 * in the heap.
 */
#define eBoilerplateAllocator(VISIT)                       \
  eGeneratableTypes(VISIT)                                 \
  VISIT(0, BuiltinCall, builtin_call)                      \
  VISIT(0, Arguments,   arguments)                         \
  VISIT(0, UnquoteExpression, unquote_expression)          \
  VISIT(0, InstantiateExpression, instantiate_expression)  \
  VISIT(0, InterpolateExpression, interpolate_expression)  \
  VISIT(0, QuoteExpression,       quote_expression)        \
  VISIT(0, Symbol, symbol)


/**
 * For each type for which a boilerplate shallow copy case can be
 * generated in image loading.
 */
#define eBoilerplateShallowCopy(VISIT)                     \
  eGeneratableTypes(VISIT)                                 \
  VISIT(0, BuiltinCall, builtin_call)                      \
  VISIT(0, UnquoteExpression, unquote_expression)          \
  VISIT(0, Channel, channel)                               \
  VISIT(0, Arguments,   arguments)                         \
  VISIT(0, InstantiateExpression, instantiate_expression)  \
  VISIT(0, InterpolateExpression, interpolate_expression)  \
  VISIT(0, QuoteExpression,       quote_expression)        \
  VISIT(0, Symbol, symbol)


#define eAcceptVisitorCases(VISIT)                         \
  eSimpleSyntaxTreeTypes(VISIT)                            \
  VISIT(0, Symbol, symbol)                                 \
  VISIT(0, Arguments, arguments)                           \
  VISIT(0, InstantiateExpression, instantiate_expression)  \
  VISIT(0, InterpolateExpression, interpolate_expression)  \
  VISIT(0, QuoteExpression,       quote_expression)        \
  VISIT(0, BuiltinCall, builtin_call)


#define eTraverseVisitorCases(VISIT)                       \
  eSimpleSyntaxTreeTypes(VISIT)                            \
  VISIT(0, Symbol, symbol)                                 \
  VISIT(0, BuiltinCall, builtin_call)


/**
 * For each type for which a boilerplate shallow fixup case can be
 * generated in image loading.
 */
#define eBoilerplateFixupShallow(VISIT)                    \
  eGeneratableTypes(VISIT)                                 \
  VISIT(0, Arguments, 0)                                   \
  VISIT(0, InstantiateExpression, 0)                       \
  VISIT(0, InterpolateExpression, 0)                       \
  VISIT(0, QuoteExpression,       0)                       \
  VISIT(0, Layout, 0)


/**
 * For each type for which a boilerplate size_in_image case can be
 * generated in image loading.
 */
#define eBoilerplateSizeInImage(VISIT)                     \
  eGeneratableTypes(VISIT)                                 \
  VISIT(0, Lambda, 0)                                      \
  VISIT(0, Layout, 0)                                      \
  VISIT(0, Context, 0)                                     \
  VISIT(0, Root, 0)                                        \
  VISIT(0, BuiltinCall, 0)                                 \
  VISIT(0, UnquoteExpression, 0)                           \
  VISIT(0, Channel, 0)                                     \
  VISIT(0, Arguments, 0)                                   \
  VISIT(0, InstantiateExpression, 0)                       \
  VISIT(0, InterpolateExpression, 0)                       \
  VISIT(0, QuoteExpression,       0)                       \
  VISIT(0, Symbol, 0)


/**
 * For each type for which a boilerplate size_in_heap case can be
 * generated in image loading.
 */
#define eBoilerplateSizeInHeap(VISIT)                      \
  eGeneratableTypes(VISIT)                                 \
  VISIT(0, Lambda, 0)                                      \
  VISIT(0, Layout, 0)                                      \
  VISIT(0, Context, 0)                                     \
  VISIT(0, BuiltinCall, 0)                                 \
  VISIT(0, UnquoteExpression, 0)                           \
  VISIT(0, Channel, 0)                                     \
  VISIT(0, Arguments, 0)                                   \
  VISIT(0, InstantiateExpression, 0)                       \
  VISIT(0, InterpolateExpression, 0)                       \
  VISIT(0, QuoteExpression,       0)                       \
  VISIT(0, Symbol, 0)


/**
 * For each type for which boilerplate accessors should be generated
 */
#define eBoilerplateAccessors(VISIT)                       \
  eGeneratableTypes(VISIT)                                 \
  VISIT(0, Layout, 0)                                      \
  VISIT(0, Lambda, 0)                                      \
  VISIT(0, Channel, 0)                                     \
  VISIT(0, Arguments, 0)                                   \
  VISIT(0, InstantiateExpression, 0)                       \
  VISIT(0, InterpolateExpression, 0)                       \
  VISIT(0, QuoteExpression,       0)                       \
  VISIT(0, Symbol, 0)


/**
 * For each type for which a boilerplate validation case can be
 * generated.
 */
#define eBoilerplateValidate(VISIT)                        \
  eGeneratableTypes(VISIT)                                 \
  VISIT(0, Channel, 0)                                     \
  VISIT(0, Layout, 0)                                      \
  VISIT(0, Context, 0)                                     \
  VISIT(0, QuoteTemplate, 0)                               \
  VISIT(0, ForwarderDescriptor, 0)                         \
  VISIT(0, Arguments, 0)                                   \
  VISIT(0, InstantiateExpression, 0)                       \
  VISIT(0, InterpolateExpression, 0)                       \
  VISIT(0, QuoteExpression,       0)                       \
  VISIT(0, Symbol, 0)


/**
 * For each type for which a boilerplate iterator case can be
 * generated in for_each_field.
 */
#define eBoilerplateIterator(VISIT)                        \
  eGeneratableTypes(VISIT)                                 \
  VISIT(0, BuiltinCall, 0)                                 \
  VISIT(0, Lambda, 0)                                      \
  VISIT(0, Layout, 0)                                      \
  VISIT(0, Channel, 0)                                     \
  VISIT(0, Arguments, 0)                                   \
  VISIT(0, InstantiateExpression, 0)                       \
  VISIT(0, InterpolateExpression, 0)                       \
  VISIT(0, QuoteExpression,       0)                       \
  VISIT(0, Symbol, 0)


// -------------------------------------
// --- I m a g e   C o n s t a n t s ---
// -------------------------------------

#define eImageObjectConsts(VISIT)                                    \
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
  VISIT(1, WhileExpression,       ConditionOffset)                   \
  VISIT(2, WhileExpression,       BodyOffset)                        \
  VISIT(3, WhileExpression,       Size)                              \
  VISIT(1, ThisExpression,        Size)                              \
  VISIT(1, SuperExpression,       ValueOffset)                       \
  VISIT(2, SuperExpression,       Size)                              \
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
  VISIT(4, LocalDefinition,       TypeOffset)                        \
  VISIT(5, LocalDefinition,       Size)                              \
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
  VISIT(1, Selector,              NameOffset)                        \
  VISIT(2, Selector,              ArgcOffset)                        \
  VISIT(3, Selector,              KeywordsOffset)                    \
  VISIT(4, Selector,              IsAccessorOffset)                  \
  VISIT(5, Selector,              Size)                              \
  VISIT(1, TaskExpression,        LambdaOffset)                      \
  VISIT(2, TaskExpression,        Size)                              \
  VISIT(1, YieldExpression,       ValueOffset)                       \
  VISIT(2, YieldExpression,       Size)                              \
  VISIT(1, Channel,               NameOffset)                        \
  VISIT(2, Channel,               Size)


// -----------------
// --- R o o t s ---
// -----------------

#define eSimpleRootObjects(VISIT)                                                                                            \
  VISIT(0,  Void,       vhoid,                         VoidValue,             new_singleton(void_layout()))                  \
  VISIT(1,  Null,       nuhll,                         NullValue,             new_singleton(null_layout()))                  \
  VISIT(2,  True,       thrue,                         TrueValue,             new_singleton(true_layout()))                  \
  VISIT(3,  False,      fahlse,                        FalseValue,            new_singleton(false_layout()))                 \
  VISIT(4,  Dictionary, toplevel,                      Toplevel,              new_dictionary())                              \
  VISIT(5,  Tuple,      empty_tuple,                   EmptyTuple,            new_tuple(0))

#define eComplicatedRootLayouts(VISIT)                                                                                       \
  VISIT(6,  Layout,     layout_layout,                 Layout,                0)

#define eSimpleRootLayouts(VISIT)                                                                                            \
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
  VISIT(50, Layout,     selector_layout,               Selector,              allocate_empty_layout(tSelector))              \
  VISIT(51, Layout,     task_expression_layout,        TaskExpression,        allocate_empty_layout(tTaskExpression))        \
  VISIT(52, Layout,     yield_expression_layout,       YieldExpression,       allocate_empty_layout(tYieldExpression))       \
  VISIT(53, Layout,     assignment_layout,             Assignment,            allocate_empty_layout(tAssignment))            \
  VISIT(54, Layout,     parameters_layout,             Parameters,            allocate_empty_layout(tParameters))            \
  VISIT(55, Layout,     channel_layout,                Channel,               allocate_empty_layout(tChannel))               \
  VISIT(56, Layout,     while_expression_layout,       WhileExpression,       allocate_empty_layout(tWhileExpression))       \
  VISIT(57, Layout,     super_expression_layout,       SuperExpression,       allocate_empty_layout(tSuperExpression))       \
  VISIT(58, Layout,     array_layout,                  Array,                 allocate_empty_layout(tArray))


#define eRoots(VISIT)                                                \
  eComplicatedRootLayouts(VISIT)                                     \
  eSimpleRoots(VISIT)

#define eSimpleRoots(VISIT)                                          \
  eSimpleRootLayouts(VISIT)                                          \
  eSimpleRootObjects(VISIT)

#define eRootLayouts(VISIT)                                          \
  eComplicatedRootLayouts(VISIT)                                     \
  eSimpleRootLayouts(VISIT)


// ---------------------------------------
// --- B u i l t - i n   M e t h o d s ---
// ---------------------------------------

/**
 * This index defines the index, layout, name and source name for all
 * internal methods.  Whenever a method is added here an associated
 * implementation must be added in builtins.cc.
 */
#define eBuiltinMethods(VISIT)                                       \
  VISIT(0,  string,              length,      "||")                  \
  VISIT(1,  string,              eq,          "=")                   \
  VISIT(2,  string,              plus,        "+")                   \
  VISIT(3,  string,              get,         "[]")                  \
  VISIT(4,  string,              hash,        "hash")                \
  VISIT(10, smi,                 plus,        "+")                   \
  VISIT(11, smi,                 minus,       "-")                   \
  VISIT(12, smi,                 times,       "×")                   \
  VISIT(13, smi,                 divide,      "÷")                   \
  VISIT(14, smi,                 abs,         "||")                  \
  VISIT(15, smi,                 less,        "<")                   \
  VISIT(16, smi,                 modulo,      "%")                   \
  VISIT(20, object,              eq,          "=")                   \
  VISIT(21, object,              to_string,   "to_string")           \
  VISIT(30, protocol_expression, evaluate,    "evaluate")            \
  VISIT(40, protocol,            new,         "new")                 \
  VISIT(41, tuple,               eq,          "=")                   \
  VISIT(46, tuple,               get,         "[]")                  \
  VISIT(47, tuple,               length,      "||")                  \
  VISIT(42, lambda,              disassemble, "disassemble")         \
  VISIT(43, lambda_expression,   params,      "parameters")          \
  VISIT(44, lambda_expression,   body,        "body")                \
  VISIT(45, channel,             send,        "send")                \
  VISIT(49, array,               get,         "[]")                  \
  VISIT(51, array,               length,      "||")                  \
  VISIT(52, array,               set,         "[]:=")


// ---------------------------------------
// --- S p e c i a l   B u i l t i n s ---
// ---------------------------------------

#define eSpecialBuiltinFunctions(VISIT)                              \
  VISIT(50, attach_task, "attach_task")


// -------------------------------------------
// --- B u i l t - i n   F u n c t i o n s ---
// -------------------------------------------

/**
 * This index defines the index, name and source name of all internal
 * functions.  Whenever a method is added here an associated
 * implementation must be added in builtins.cc.
 */
#define ePlainBuiltinFunctions(VISIT)                                \
  VISIT(101, raw_print,          "raw_print")                        \
  VISIT(102, compile_expression, "compile_expression")               \
  VISIT(103, lift,               "lift")                             \
  VISIT(104, new_forwarder,      "new_forwarder")                    \
  VISIT(105, set_target,         "set_target")                       \
  VISIT(109, close,              "close")                            \
  VISIT(106, is_whitespace,      "whitespace?")                      \
  VISIT(107, is_alpha,           "alpha?")                           \
  VISIT(108, is_digit,           "digit?")                           \
  VISIT(110, new_array,          "new_array")


#define eBuiltinFunctions(VISIT)                                     \
  eSpecialBuiltinFunctions(VISIT)                                    \
  ePlainBuiltinFunctions(VISIT)


#endif // _UTILS_CONSTS
