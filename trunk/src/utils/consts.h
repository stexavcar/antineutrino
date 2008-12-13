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
  VISIT(1,  HashMap,               hash_map)               \
  VISIT(2,  Method,                method)                 \
  VISIT(3,  Protocol,              protocol)               \
  VISIT(4,  Task,                  task)                   \
  VISIT(5,  Signature,             signature)              \
  VISIT(6,  Selector,              selector)               \
  VISIT(7,  Cell,                  cell)


#define eObjectTypes(VISIT)                                \
  VISIT(8,  Layout,                layout)                 \
  VISIT(9,  String,                string)                 \
  VISIT(10, Tuple,                 tuple)                  \
  VISIT(11, Array,                 array)                  \
  VISIT(12, Void,                  void)                   \
  VISIT(13, Null,                  null)                   \
  VISIT(14, True,                  true)                   \
  VISIT(15, False,                 false)                  \
  VISIT(16, Lambda,                lambda)                 \
  VISIT(17, Buffer,                buffer)                 \
  VISIT(18, Code,                  code)                   \
  VISIT(19, Instance,              instance)               \
  VISIT(20, Stack,                 stack)                  \
  VISIT(21, Context,               context)                \
  VISIT(22, ForwarderDescriptor,   forwarder_descriptor)   \
  VISIT(23, Channel,               channel)                \
  VISIT(24, Symbol,                symbol)                 \
  eGeneratableObjectTypes(VISIT)                           \
  eSyntaxTreeTypes(VISIT)

#define ePlainVirtualTypes(VISIT)                          \
  VISIT(25, Object,                object)                 \
  VISIT(26, Value,                 0)                      \
  VISIT(27, AbstractBuffer,        0)                      \
  VISIT(28, AbstractTuple,         0)                      \
  VISIT(29, Singleton,             0)                      \
  VISIT(30, SyntaxTree,            0)                      \
  VISIT(31, Immediate,             0)                      \
  VISIT(32, Bool,                  0)

#define eVirtualTypes(VISIT)                               \
  eSpecialTypes(VISIT)                                     \
  ePlainVirtualTypes(VISIT)

#define eSpecialTypes(VISIT)                               \
  VISIT(33, Root,                  0)                      \

#define eSignalTypes(VISIT)                                \
  VISIT(40, AllocationFailed,      0)                      \
  VISIT(41, FatalError,            0)                      \
  VISIT(42, Nothing,               0)                      \
  VISIT(43, TypeMismatch,          0)                      \
  VISIT(44, StackOverflow,         0)                      \
  VISIT(45, Success,               0)

#define eNormalTypes(VISIT)                                \
  VISIT(50, Smi,                   smi)                    \
  VISIT(53, Forwarder,             0)                      \
  ePlainVirtualTypes(VISIT)                                \
  eObjectTypes(VISIT)


#define eValueTypes(VISIT)                                 \
  VISIT(51, Signal,                0)                      \
  VISIT(52, ForwardPointer,        0)                      \
  eNormalTypes(VISIT)                                      \
  eSignalTypes(VISIT)

#define eLayoutTypes(VISIT)                                \
  VISIT(90, SimpleLayout,          0)                      \
  VISIT(91, InstanceLayout,        0)

#define eDeclaredTypes(VISIT)                              \
  eValueTypes(VISIT)                                       \
  eSpecialTypes(VISIT)                                     \
  eLayoutTypes(VISIT)


#define eSyntaxTreeTypes(VISIT)                            \
  VISIT(54, BuiltinCall,           builtin_call)           \
  VISIT(55, Arguments,             arguments)              \
  VISIT(56, InstantiateExpression, instantiate_expression) \
  VISIT(57, InterpolateExpression, interpolate_expression) \
  VISIT(58, Parameters,            parameters)             \
  eSimpleSyntaxTreeTypes(VISIT)


#define eSimpleSyntaxTreeTypes(VISIT)                      \
  VISIT(59, LiteralExpression,     literal_expression)     \
  VISIT(60, InvokeExpression,      invoke_expression)      \
  VISIT(61, ProtocolExpression,    protocol_expression)    \
  VISIT(62, ReturnExpression,      return_expression)      \
  VISIT(63, MethodExpression,      method_expression)      \
  VISIT(64, SequenceExpression,    sequence_expression)    \
  VISIT(65, TupleExpression,       tuple_expression)       \
  VISIT(66, GlobalVariable,        global_variable)        \
  VISIT(67, CallExpression,        call_expression)        \
  VISIT(68, ConditionalExpression, conditional_expression) \
  VISIT(69, ThisExpression,        this_expression)        \
  VISIT(70, LambdaExpression,      lambda_expression)      \
  VISIT(71, LocalDefinition,       local_definition)       \
  VISIT(72, RaiseExpression,       raise_expression)       \
  VISIT(73, OnClause,              on_clause)              \
  VISIT(74, DoOnExpression,        do_on_expression)       \
  VISIT(75, TaskExpression,        task_expression)        \
  VISIT(76, YieldExpression,       yield_expression)       \
  VISIT(77, Assignment,            assignment)             \
  VISIT(78, WhileExpression,       while_expression)       \
  VISIT(79, SuperExpression,       super_expression)       \
  VISIT(80, LocalVariable,         local_variable)


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
  VISIT(0, InstantiateExpression, instantiate_expression)  \
  VISIT(0, InterpolateExpression, interpolate_expression)  \
  VISIT(0, Parameters,            parameters)              \
  VISIT(0, Symbol, symbol)


/**
 * For each type for which a boilerplate shallow copy case can be
 * generated in image loading.
 */
#define eBoilerplateShallowCopy(VISIT)                     \
  eGeneratableTypes(VISIT)                                 \
  VISIT(0, BuiltinCall, builtin_call)                      \
  VISIT(0, Channel, channel)                               \
  VISIT(0, Arguments,   arguments)                         \
  VISIT(0, InstantiateExpression, instantiate_expression)  \
  VISIT(0, InterpolateExpression, interpolate_expression)  \
  VISIT(0, Parameters,            parameters)              \
  VISIT(0, Symbol, symbol)


#define eAcceptVisitorCases(VISIT)                         \
  eSimpleSyntaxTreeTypes(VISIT)                            \
  VISIT(0, Arguments, arguments)                           \
  VISIT(0, InstantiateExpression, instantiate_expression)  \
  VISIT(0, InterpolateExpression, interpolate_expression)  \
  VISIT(0, Parameters,            parameters)              \
  VISIT(0, BuiltinCall, builtin_call)


#define eTraverseVisitorCases(VISIT)                       \
  eSimpleSyntaxTreeTypes(VISIT)                            \
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
  VISIT(0, Parameters,            0)                       \
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
  VISIT(0, Channel, 0)                                     \
  VISIT(0, Arguments, 0)                                   \
  VISIT(0, InstantiateExpression, 0)                       \
  VISIT(0, InterpolateExpression, 0)                       \
  VISIT(0, Parameters,            0)                       \
  VISIT(0, Symbol, 0)


/**
 * For each type for which a boilerplate size_in_heap case can be
 * generated in image loading.
 */
#define eBoilerplateSizeInHeap(VISIT)                      \
  eGeneratableTypes(VISIT)                                 \
  VISIT(0, Lambda, 0)                                      \
  VISIT(0, Context, 0)                                     \
  VISIT(0, BuiltinCall, 0)                                 \
  VISIT(0, Channel, 0)                                     \
  VISIT(0, Arguments, 0)                                   \
  VISIT(0, InstantiateExpression, 0)                       \
  VISIT(0, InterpolateExpression, 0)                       \
  VISIT(0, Parameters,            0)                       \
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
  VISIT(0, Parameters,            0)                       \
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
  VISIT(0, ForwarderDescriptor, 0)                         \
  VISIT(0, Arguments, 0)                                   \
  VISIT(0, InstantiateExpression, 0)                       \
  VISIT(0, InterpolateExpression, 0)                       \
  VISIT(0, Parameters,            0)                       \
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
  VISIT(0, Parameters,            0)                       \
  VISIT(0, Symbol, 0)


// -------------------------------------
// --- I m a g e   C o n s t a n t s ---
// -------------------------------------

#define eImageObjectConsts(VISIT)                                    \
  VISIT(0, Object,                LayoutOffset)                      \
  VISIT(1, Object,                HeaderSize)                        \
  VISIT(1, HashMap,            TableOffset)                       \
  VISIT(2, HashMap,            Size)                              \
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
  VISIT(1, GlobalVariable,        NameOffset)                        \
  VISIT(2, GlobalVariable,        Size)                              \
  VISIT(1, LocalVariable,         SymbolOffset)                      \
  VISIT(2, LocalVariable,         Size)                              \
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
  VISIT(1, LambdaExpression,      ParametersOffset)                  \
  VISIT(2, LambdaExpression,      BodyOffset)                        \
  VISIT(3, LambdaExpression,      IsLocalOffset)                     \
  VISIT(4, LambdaExpression,      Size)                              \
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
  VISIT(1, Task,                  StackOffset)                       \
  VISIT(2, Task,                  CallerOffset)                      \
  VISIT(3, Task,                  Size)                              \
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
  VISIT(2, Channel,               Size)                              \
  VISIT(1, Cell,                  ValueOffset)                       \
  VISIT(2, Cell,                  Size)


// -----------------
// --- R o o t s ---
// -----------------

#define eSimpleRootObjects(VISIT)                                    \
  eSimpleExternalRootObjects(VISIT)                                  \
  eSimpleInternalRootObjects(VISIT)

#define eSimpleExternalRootObjects(VISIT)                                                                                    \
  VISIT(0,  Void,       vhoid,                         VoidValue,             new_singleton<Void>(void_layout()))                  \
  VISIT(1,  Null,       nuhll,                         NullValue,             new_singleton<Null>(null_layout()))                  \
  VISIT(2,  True,       thrue,                         TrueValue,             new_singleton<True>(true_layout()))                  \
  VISIT(3,  False,      fahlse,                        FalseValue,            new_singleton<False>(false_layout()))                 \
  VISIT(4,  HashMap, toplevel,                         Toplevel,              new_hash_map())

#define eComplicatedRootLayouts(VISIT)                                                                                       \
  VISIT(5,  Layout,     layout_layout,                 Layout,                0)

#define eSimpleExternalRootLayouts(VISIT)                                                                                    \
  VISIT(6,  SimpleLayout,     protocol_layout,               Protocol,              allocate_empty_simple_layout(tProtocol))              \
  VISIT(7,  SimpleLayout,     string_layout,                 String,                allocate_empty_simple_layout(tString))                \
  VISIT(8,  SimpleLayout,     tuple_layout,                  Tuple,                 allocate_empty_simple_layout(tTuple))                 \
  VISIT(9,  SimpleLayout,     void_layout,                   Void,                  allocate_empty_simple_layout(tVoid))                  \
  VISIT(10, SimpleLayout,     null_layout,                   Null,                  allocate_empty_simple_layout(tNull))                  \
  VISIT(11, SimpleLayout,     true_layout,                   True,                  allocate_empty_simple_layout(tTrue))                  \
  VISIT(12, SimpleLayout,     false_layout,                  False,                 allocate_empty_simple_layout(tFalse))                 \
  VISIT(13, SimpleLayout,     literal_expression_layout,     LiteralExpression,     allocate_empty_simple_layout(tLiteralExpression))     \
  VISIT(14, SimpleLayout,     hash_map_layout,               HashMap,               allocate_empty_simple_layout(tHashMap))            \
  VISIT(15, SimpleLayout,     lambda_layout,                 Lambda,                allocate_empty_simple_layout(tLambda))                \
  VISIT(16, SimpleLayout,     buffer_layout,                 Buffer,                allocate_empty_simple_layout(tBuffer))                \
  VISIT(17, SimpleLayout,     code_layout,                   Code,                  allocate_empty_simple_layout(tCode))                  \
  VISIT(18, SimpleLayout,     method_layout,                 Method,                allocate_empty_simple_layout(tMethod))                \
  VISIT(19, SimpleLayout,     smi_layout,                    SmallInteger,          allocate_empty_simple_layout(tSmi))                   \
  VISIT(20, SimpleLayout,     invoke_expression_layout,      InvokeExpression,      allocate_empty_simple_layout(tInvokeExpression))      \
  VISIT(21, SimpleLayout,     protocol_expression_layout,    ProtocolExpression,    allocate_empty_simple_layout(tProtocolExpression))    \
  VISIT(22, SimpleLayout,     return_expression_layout,      ReturnExpression,      allocate_empty_simple_layout(tReturnExpression))      \
  VISIT(23, SimpleLayout,     method_expression_layout,      MethodExpression,      allocate_empty_simple_layout(tMethodExpression))      \
  VISIT(24, SimpleLayout,     sequence_expression_layout,    SequenceExpression,    allocate_empty_simple_layout(tSequenceExpression))    \
  VISIT(25, SimpleLayout,     tuple_expression_layout,       TupleExpression,       allocate_empty_simple_layout(tTupleExpression))       \
  VISIT(26, SimpleLayout,     global_variable_layout,        GlobalVariable,        allocate_empty_simple_layout(tGlobalVariable))        \
  VISIT(27, SimpleLayout,     symbol_layout,                 Symbol,                allocate_empty_simple_layout(tSymbol))                \
  VISIT(28, SimpleLayout,     call_expression_layout,        CallExpression,        allocate_empty_simple_layout(tCallExpression))        \
  VISIT(29, SimpleLayout,     conditional_expression_layout, ConditionalExpression, allocate_empty_simple_layout(tConditionalExpression)) \
  VISIT(30, SimpleLayout,     this_expression_layout,        ThisExpression,        allocate_empty_simple_layout(tThisExpression))        \
  VISIT(31, SimpleLayout,     lambda_expression_layout,      LambdaExpression,      allocate_empty_simple_layout(tLambdaExpression))      \
  VISIT(32, SimpleLayout,     builtin_call_layout,           BuiltinCall,           allocate_empty_simple_layout(tBuiltinCall))           \
  VISIT(33, SimpleLayout,     interpolate_expression_layout, InterpolateExpression, allocate_empty_simple_layout(tInterpolateExpression)) \
  VISIT(34, SimpleLayout,     local_definition_layout,       LocalDefinition,       allocate_empty_simple_layout(tLocalDefinition))       \
  VISIT(35, SimpleLayout,     stack_layout,                  Stack,                 allocate_empty_simple_layout(tStack))                 \
  VISIT(36, SimpleLayout,     task_layout,                   Task,                  allocate_empty_simple_layout(tTask))                  \
  VISIT(37, SimpleLayout,     raise_expression_layout,       RaiseExpression,       allocate_empty_simple_layout(tRaiseExpression))       \
  VISIT(38, SimpleLayout,     on_clause_layout,              OnClause,              allocate_empty_simple_layout(tOnClause))              \
  VISIT(39, SimpleLayout,     do_on_expression_layout,       DoOnExpression,        allocate_empty_simple_layout(tDoOnExpression))        \
  VISIT(40, SimpleLayout,     instantiate_expression_layout, InstantiateExpression, allocate_empty_simple_layout(tInstantiateExpression)) \
  VISIT(41, SimpleLayout,     context_layout,                Context,               allocate_empty_simple_layout(tContext))               \
  VISIT(42, SimpleLayout,     signature_layout,              Signature,             allocate_empty_simple_layout(tSignature))             \
  VISIT(43, SimpleLayout,     forwarder_descriptor_layout,   ForwarderDescriptor,   allocate_empty_simple_layout(tForwarderDescriptor))   \
  VISIT(44, SimpleLayout,     arguments_layout,              Arguments,             allocate_empty_simple_layout(tArguments))             \
  VISIT(45, SimpleLayout,     selector_layout,               Selector,              allocate_empty_simple_layout(tSelector))              \
  VISIT(46, SimpleLayout,     task_expression_layout,        TaskExpression,        allocate_empty_simple_layout(tTaskExpression))        \
  VISIT(47, SimpleLayout,     yield_expression_layout,       YieldExpression,       allocate_empty_simple_layout(tYieldExpression))       \
  VISIT(48, SimpleLayout,     assignment_layout,             Assignment,            allocate_empty_simple_layout(tAssignment))            \
  VISIT(49, SimpleLayout,     parameters_layout,             Parameters,            allocate_empty_simple_layout(tParameters))            \
  VISIT(50, SimpleLayout,     channel_layout,                Channel,               allocate_empty_simple_layout(tChannel))               \
  VISIT(51, SimpleLayout,     while_expression_layout,       WhileExpression,       allocate_empty_simple_layout(tWhileExpression))       \
  VISIT(52, SimpleLayout,     super_expression_layout,       SuperExpression,       allocate_empty_simple_layout(tSuperExpression))       \
  VISIT(53, SimpleLayout,     array_layout,                  Array,                 allocate_empty_simple_layout(tArray))                 \
  VISIT(54, SimpleLayout,     local_variable_layout,         LocalVariable,         allocate_empty_simple_layout(tLocalVariable))         \
  VISIT(55, SimpleLayout,     cell_layout,                   Cell,                  allocate_empty_simple_layout(tCell))

#define eSimpleInternalRootObjects(VISIT)                                                                                    \
  VISIT(56, Tuple,      empty_tuple,                   EmptyTuple,            new_tuple(0))                                  \
  VISIT(57, Symbol,     this_symbol,                   ThisSymbol,            new_symbol(vhoid()))                           \
  VISIT(58, Context,    dummy_context,                 DummyContext,          new_context())

#define eInternalRoots(VISIT)                                        \
  eSimpleInternalRootObjects(VISIT)

#define eRoots(VISIT)                                                \
  eExternalRoots(VISIT)                                              \
  eInternalRoots(VISIT)

#define eExternalRoots(VISIT)                                        \
  eComplicatedRootLayouts(VISIT)                                     \
  eSimpleExternalRoots(VISIT)

#define eSimpleExternalRoots(VISIT)                                  \
  eSimpleExternalRootLayouts(VISIT)                                  \
  eSimpleExternalRootObjects(VISIT)

#define eSimpleRoots(VISIT)                                          \
  eSimpleExternalRoots(VISIT)                                        \
  eSimpleInternalRootObjects(VISIT)

#define eRootLayouts(VISIT)                                          \
  eComplicatedRootLayouts(VISIT)                                     \
  eSimpleExternalRootLayouts(VISIT)


// ---------------------------------------
// --- B u i l t - i n   M e t h o d s ---
// ---------------------------------------

/**
 * This index defines the index, layout, name and source name for all
 * internal methods.  Whenever a method is added here an associated
 * implementation must be added in builtins.cc.
 */
#define eBuiltinMethods(VISIT)                                       \
  VISIT(0,  string,                 length,      "||")               \
  VISIT(1,  string,                 eq,          "=")                \
  VISIT(2,  string,                 plus,        "+")                \
  VISIT(3,  string,                 get,         "[]")               \
  VISIT(4,  string,                 hash,        "hash")             \
  VISIT(10, smi,                    plus,        "+")                \
  VISIT(11, smi,                    minus,       "-")                \
  VISIT(12, smi,                    times,       "×")                \
  VISIT(13, smi,                    divide,      "÷")                \
  VISIT(14, smi,                    abs,         "||")               \
  VISIT(15, smi,                    less,        "<")                \
  VISIT(16, smi,                    modulo,      "%")                \
  VISIT(20, object,                 eq,          "=")                \
  VISIT(21, object,                 to_string,   "to_string")        \
  VISIT(30, protocol_expression,    evaluate,    "evaluate")         \
  VISIT(40, protocol,               new,         "new")              \
  VISIT(41, tuple,                  eq,          "=")                \
  VISIT(46, tuple,                  get,         "[]")               \
  VISIT(47, tuple,                  length,      "||")               \
  VISIT(42, lambda,                 disassemble, "disassemble")      \
  VISIT(43, lambda_expression,      params,      "parameters")       \
  VISIT(44, lambda_expression,      body,        "body")             \
  VISIT(49, array,                  get,         "[]")               \
  VISIT(51, array,                  length,      "||")               \
  VISIT(52, array,                  set,         "[]:=")             \
  \
  VISIT(60, invoke_expression,      new,         "new")              \
  VISIT(61, selector,               new,         "new")              \
  VISIT(62, literal_expression,     new,         "new")              \
  VISIT(63, arguments,              new,         "new")              \
  VISIT(64, protocol_expression,    new,         "new")              \
  VISIT(65, parameters,             new,         "new")              \
  VISIT(66, return_expression,      new,         "new")              \
  VISIT(67, lambda_expression,      new,         "new")              \
  VISIT(68, method_expression,      new,         "new")              \
  VISIT(69, sequence_expression,    new,         "new")              \
  VISIT(70, tuple_expression,       new,         "new")              \
  VISIT(71, global_variable,        new,         "new")              \
  VISIT(72, call_expression,        new,         "new")              \
  VISIT(73, local_variable,         new,         "new")              \
  VISIT(74, conditional_expression, new,         "new")              \
  VISIT(75, this_expression,        new,         "new")


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
  VISIT(110, new_array,          "new_array")                        \
  VISIT(111, process_unquote,    "process_unquote")


#define eBuiltinFunctions(VISIT)                                     \
  eSpecialBuiltinFunctions(VISIT)                                    \
  ePlainBuiltinFunctions(VISIT)


#endif // _UTILS_CONSTS
