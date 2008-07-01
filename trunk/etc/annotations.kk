def @check ${Type:\w+} \* ${name:[\w_]+} = ${operation:[^;]+}
  KLIDKIKS_CHECK_OBJ(${Type}, ${name}, ${operation})

def @check(likelihood=maybe) ref < ${Type:\w+} > ${name:\w+} = ${operation:[^;]+}
  KLIDKIKS_CHECK_REF(${likelihood}, ${Type}, ${name}, ${operation})

def @alloc(Result=${Type}) ${Type:\w+} \* ${name:\w+} = ${operation:[^;]+}
  KLIDKIKS_ALLOC(${Result}, ${Type}, ${name}, ${operation})

def @try(likelihood=possibly) ${operation:[^;]+}
  KLIDKIKS_TRY(${likelihood}, ${operation}, ${i})
