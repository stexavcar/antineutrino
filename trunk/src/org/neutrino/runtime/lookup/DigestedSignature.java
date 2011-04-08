package org.neutrino.runtime.lookup;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.javatrino.ast.Pattern;
import org.neutrino.runtime.RValue;

public class DigestedSignature {

  private final List<ParameterTag> entries;

  public DigestedSignature(List<ParameterTag> entries) {
    this.entries = entries;
  }

  public List<ParameterTag> getEntries() {
    return entries;
  }

  @Override
  public String toString() {
    return String.valueOf(entries);
  }

  public static class ParameterTag implements Comparable<ParameterTag> {

    public final int index;
    public final RValue name;

    public ParameterTag(int index, RValue name) {
      this.index = index;
      this.name = name;
    }

    @Override
    public int compareTo(ParameterTag that) {
      return this.name.compareTo(that.name);
    }

    @Override
    public String toString() {
      return name + "(" + index + ")";
    }

  }

  public static DigestedSignature create(List<Pattern> signature) {
    List<ParameterTag> entries = new ArrayList<ParameterTag>();
    for (int i = 0; i < signature.size(); i++) {
      Pattern pat = signature.get(i);
      for (RValue tag : pat.tags)
        entries.add(new ParameterTag(i, tag));
    }
    Collections.sort(entries);
    return new DigestedSignature(entries);
  }

}
