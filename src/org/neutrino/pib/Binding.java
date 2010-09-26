package org.neutrino.pib;

import java.util.List;

import org.neutrino.plankton.ISeedable;
import org.neutrino.plankton.annotations.Growable;
import org.neutrino.plankton.annotations.SeedMember;
import org.neutrino.syntax.Annotation;

/**
 * A piece of code include bytecode and literals.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
@Growable(Binding.TAG)
public class Binding implements ISeedable {

  public static final String ENTRY_POINT_ANNOTATION = "entry_point";
  public static final String TAG = "org::neutrino::pib::Binding";

  public @SeedMember("annotations") List<Annotation> annots;
  public @SeedMember CodeBundle code;

  public Binding(List<Annotation> annots, CodeBundle code) {
    this.annots = annots;
    this.code = code;
  }

  public Binding() { }

  public Annotation getAnnotation(String tag) {
    for (Annotation annot : this.annots) {
      if (annot.getTag().equals(tag))
        return annot;
    }
    return null;
  }

  public CodeBundle getCode() {
    return this.code;
  }

  @Override
  public String toString() {
    return "a Binding {annots=" + annots + ", code=" + code + "}";
  }

}
