package org.neutrino.pib;

import java.util.List;

import org.neutrino.plankton.ISeedable;
import org.neutrino.plankton.annotations.Growable;
import org.neutrino.plankton.annotations.SeedMember;

@Growable(CodeBundle.TAG)
public class CodeBundle implements ISeedable {

  public static final String TAG = "org::neutrino::pib::CodeBundle";

  public @SeedMember byte[] code;
  public @SeedMember List<Object> literals;

  public CodeBundle(byte[] code, List<Object> literals) {
    this.code = code;
    this.literals = literals;
  }

  public CodeBundle() { }

  public byte[] getCode() {
    return this.code;
  }

  public List<Object> getLiterals() {
    return this.literals;
  }

}
