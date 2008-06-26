def @check ${Type:\w+} \* ${name:\w+} = ${operation:[^;]+}
  Data *__${name}_val__ = ${operation};
  if (is<Signal>(__${name}_val__)) return cast<Signal>(__${name}_val__);
  ${Type}* ${name} = cast<${Type}>(__${name}_val__);

def @check ref < ${Type:\w+} > ${name:\w+} = ${operation:[^;]+}
  Data *__${name}_val__ = ${operation};
  if (is<Signal>(__${name}_val__)) return cast<Signal>(__${name}_val__);
  ref<${Type}> ${name} = refs().new_ref(cast<${Type}>(__${name}_val__));
