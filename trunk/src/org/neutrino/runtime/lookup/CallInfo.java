package org.neutrino.runtime.lookup;

import java.util.List;

import org.neutrino.runtime.RValue;
/**
 * Information about a call site.  Used for doing method lookup.
 */
public class CallInfo {

  /**
   * Description of a single argument.
   */
  public static class ArgumentEntry implements Comparable<ArgumentEntry>{

    public final RValue tag;
    private final RValue staticValue;
    public final int index;

    public ArgumentEntry(RValue tag, RValue staticValue, int index) {
      this.tag = tag;
      this.staticValue = staticValue;
      this.index = index;
    }

    @Override
    public int compareTo(ArgumentEntry that) {
      return tag.compareTo(that.tag);
    }

    @Override
    public String toString() {
      return index + ": " + tag;
    }

    @Override
    public int hashCode() {
      return tag.hashCode() ^ (staticValue == null ? 0 : staticValue.hashCode()) ^ index;
    }

    @Override
    public boolean equals(Object other) {
      if (!(other instanceof ArgumentEntry))
        return false;
      ArgumentEntry that = (ArgumentEntry) other;
      if (!tag.equals(that.tag))
        return false;
      if (staticValue != null && that.staticValue != null && !staticValue.equals(that.staticValue))
        return false;
      if (staticValue != that.staticValue)
        return false;
      return this.index == that.index;
    }

  }

  public final int primaryArgument;
  public final List<ArgumentEntry> entries;

  public CallInfo(int primaryArgument, List<ArgumentEntry> entries) {
    this.primaryArgument = primaryArgument;
    this.entries = entries;
  }

  public int getArgumentCount() {
    return entries.size();
  }

  @Override
  public String toString() {
    return String.valueOf(entries);
  }

  @Override
  public int hashCode() {
    return primaryArgument ^ entries.hashCode();
  }

  @Override
  public boolean equals(Object that) {
    if (!(that instanceof CallInfo)) {
      return false;
    }
    CallInfo info = (CallInfo) that;
    return this.primaryArgument == info.primaryArgument
        && this.entries.equals(info.entries);
  }

}
