def @check ${Type:\w+} \* ${name:[\w_]+} = ${operation:[^;]+}
  Data *__${name}_val__ = ${operation};
  if (is<Signal>(__${name}_val__)) return cast<Signal>(__${name}_val__);
  ${Type}* ${name} = cast<${Type}>(__${name}_val__);

def @protect ref < ${Type:\w+} > ${name:\w+} = ${operation:[^;]+}
  Data *__${name}_val__ = ${operation};
  if (is<Signal>(__${name}_val__)) return cast<Signal>(__${name}_val__);
  ref<${Type}> ${name} = safe(cast<${Type}>(__${name}_val__));

def @alloc ${Type:\w+} \* ${name:\w+} = ${operation:[^;]+}
  Data *__${name}_val__ = ${operation};
  if (is<AllocationFailed>(__${name}_val__)) return cast<AllocationFailed>(__${name}_val__);
  ${Type}* ${name} = cast<${Type}>(__${name}_val__);

def @try ${operation:[^;]+}
  do {
    Signal *__try_${i}__ = ${operation};
    if (!is<Success>(__try_${i}__)) return __try_${i}__;
  } while (false)
