package org.neutrino.oldton;

/**
 * Abstract superclass for plankton values.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
public interface PValue {

  /**
   * Type tags used to identify the different types of values.
   */
  public enum Type {

    STRING(Oldton.kStringTag), MAP(Oldton.kMapTag),
    BLOB(Oldton.kBlobTag), INTEGER(Oldton.kIntegerTag),
    ARRAY(Oldton.kArrayTag), SEED(Oldton.kSeedTag),
    BOOL(Oldton.kBoolTag);

    private final int tag;

    private Type(int tag) {
      this.tag = tag;
    }

    public int getTag() {
      return this.tag;
    }

  }

  public Type getType();

}
