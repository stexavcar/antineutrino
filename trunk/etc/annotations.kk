def @check ${Type:\w+} \* ${name:[\w_]+} = ${operation:[^;]+}
  KLIDKIKS_CHECK_OBJ(${Type}, ${name}, ${operation})

def @check ref < ${Type:\w+} > ${name:\w+} = ${operation:[^;]+}
  maybe<${Type}> __${name}_opt__ = ${operation};
  if (__${name}_opt__.has_failed()) return __${name}_opt__.signal();
  ref<${Type}> ${name} = protect(__${name}_opt__.value());

def @alloc ${Type:\w+} \* ${name:\w+} = ${operation:[^;]+}
  KLIDKIKS_ALLOC(${Type}, ${name}, ${operation})

def @objalloc ${Type:\w+} \* ${name:\w+} = ${operation:[^;]+}
  KLIDKIKS_OBJALLOC(${Type}, ${name}, ${operation})

def @bufalloc ${Type:\w+} \* ${name:\w+} = ${operation:[^;]+}
  KLIDKIKS_BUFALLOC(${Type}, ${name}, ${operation})

def @try ${operation:[^;]+}
  do {
    Signal *__try_${i}__ = ${operation};
    if (!is<Success>(__try_${i}__)) return __try_${i}__;
  } while (false)
