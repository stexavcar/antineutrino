def @check ${Type:\w+} \* ${name:[\w_]+} = ${operation:[^;]+}
  KLIDKIKS_CHECK_OBJ(${Type}, ${name}, ${operation})

def @check ref < ${Type:\w+} > ${name:\w+} = ${operation:[^;]+}
  KLIDKIKS_CHECK_REF(${Type}, ${name}, ${operation})

def @alloc(Result=${Type}) ${Type:\w+} \* ${name:\w+} = ${operation:[^;]+}
  KLIDKIKS_ALLOC(${Result}, ${Type}, ${name}, ${operation})

def @try ${operation:[^;]+}
  KLIDKIKS_TRY(${operation}, ${i})
