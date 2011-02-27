package org.neutrino.syntax;

import java.util.List;

import org.neutrino.plankton.Store;
import org.neutrino.runtime.RValue;

public class Annotation {

  public @Store String tag;
  public @Store List<RValue> args;

  public Annotation(String tag, List<RValue> args) {
    this.tag = tag;
    this.args = args;
  }

  public Annotation() { }

  public String getTag() {
    return this.tag;
  }

  public List<RValue> getArguments() {
    return this.args;
  }

  public RValue getArgument(int index) {
    return this.args.get(index);
  }

  @Override
  public String toString() {
    return tag + args;
  }

  public static Annotation get(String name, List<Annotation> annots) {
    for (Annotation annot : annots) {
      if (annot.tag.equals(name))
        return annot;
    }
    return null;
  }

}
