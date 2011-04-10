package org.neutrino.runtime.lookup;

import java.util.List;

import org.javatrino.ast.Method;
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
    public final RValue staticValue;
    public final int index;

    public ArgumentEntry(RValue tag, RValue staticValue, int index) {
      this.tag = tag;
      this.staticValue = staticValue;
      this.index = index;
    }

    @Override
    public int compareTo(ArgumentEntry that) {
      int tagDiff = tag.compareTo(that.tag);
      if (tagDiff != 0)
        return tagDiff;
      int indexDiff = this.index - that.index;
      if (indexDiff != 0)
        return indexDiff;
      if (staticValue != that.staticValue) {
        if (staticValue == null) {
          return 1;
        } else if (that.staticValue == null) {
          return -1;
        } else {
          return this.staticValue.compareTo(that.staticValue);
        }
      } else {
        return 0;
      }
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
  public List<Method> candidates;

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
