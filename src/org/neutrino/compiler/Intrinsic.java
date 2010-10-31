package org.neutrino.compiler;

import org.neutrino.syntax.Tree;
import org.neutrino.syntax.Tree.Call;
import org.neutrino.syntax.Tree.Expression.Type;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.reflect.Field;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public abstract class Intrinsic {

  public static final String ANNOTATION = "intrinsic";

  public static Intrinsic get(String id) {
    return INTRINSICS.get(id);
  }

  public abstract boolean isApplicable(Call that);

  public abstract void generate(Call that, CodeGenerator codeGenerator);

  @Retention(RetentionPolicy.RUNTIME)
  private static @interface Marker {
    public String value();
  }

  @Marker("if") static final Intrinsic IF = new Intrinsic() {

    @Override
    public boolean isApplicable(Call that) {
      if (true)
        return false;
      List<Tree.Expression> args = that.getArguments();
      if (args.size() != 4)
        return false;
      return args.get(2).is(Type.LAMBDA) && args.get(3).is(Type.LAMBDA);
    }

    @Override
    public void generate(Call that, CodeGenerator codegen) {
      /*
      List<Tree.Expression> args = that.getArguments();
      Tree.New.Field thenField = ((Tree.New) args.get(2)).getField(Lambda.NAME);
      Tree.New.Field elseField = ((Tree.New) args.get(3)).getField(Lambda.NAME);
      codegen.generateIf(args.get(1), thenField.getBody(),
          elseField.getBody());
       */
    }
  };

  @Marker("repeat") static final Intrinsic REPEAT = new Intrinsic() {

    @Override
    public boolean isApplicable(Call that) {
      return false;
    }

    @Override
    public void generate(Call that, CodeGenerator codegen) {
    }

  };

  private static final Map<String, Intrinsic> INTRINSICS = new HashMap<String, Intrinsic>() {{
    try {
      for (Field field : Intrinsic.class.getDeclaredFields()) {
        Marker marker = field.getAnnotation(Marker.class);
        if (marker != null) {
          String value = marker.value();
          put(value, (Intrinsic) field.get(null));
        }
      }
    } catch (IllegalAccessException iae) {
      throw new RuntimeException(iae);
    }
  }};

}
