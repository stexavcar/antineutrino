package org.neutrino.syntax;

import java.util.List;

import org.neutrino.plankton.ISeedable;
import org.neutrino.plankton.annotations.Growable;
import org.neutrino.plankton.annotations.SeedMember;
import org.neutrino.runtime.RValue;

@Growable(Annotation.TAG)
public class Annotation implements ISeedable {

  static final String TAG = "org::neutrino::syntax::Annotation";
  public @SeedMember String tag;
  public @SeedMember List<RValue> args;

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

}
