package org.neutrino.pib;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.neutrino.plankton.ISeedable;
import org.neutrino.plankton.PSeed;
import org.neutrino.plankton.PString;
import org.neutrino.plankton.PValue;
import org.neutrino.plankton.Plankton;
import org.neutrino.plankton.annotations.Growable;
import org.neutrino.plankton.annotations.SeedMember;

/**
 * A piece of code include bytecode and literals.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
@Growable(Binding.TAG)
public class Binding implements ISeedable {

  public static final String ENTRY_POINT_ANNOTATION = "entry_point";
  public static final String TAG = "org::neutrino::pib::Binding";

  public @SeedMember("annotations") List<String> annots;
  public @SeedMember byte[] code;

  public Binding(List<String> annots, byte[] code) {
    this.annots = annots;
    this.code = code;
  }

  public Binding() { }

  public List<String> getAnnotations() {
    return this.annots;
  }

  public PSeed toSeed(Plankton plankton) {
    List<PString> pAnnots = new ArrayList<PString>();
    for (String annot : annots)
      pAnnots.add(Plankton.newString(annot));
    Map<PString, PValue> map = new HashMap<PString, PValue>();
    map.put(Plankton.newString("code"), Plankton.newBlob(code));
    map.put(Plankton.newString("annotations"), Plankton.newArray(pAnnots));
    return plankton.newSeed(TAG, Plankton.newMap(map));
  }

  @Override
  public String toString() {
    return "a Binding {annots=" + annots + ", code=" + code + "}";
  }

}
