package org.neutrino.plankton;

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

    STRING(Plankton.kStringTag), MAP(Plankton.kMapTag),
    BLOB(Plankton.kBlobTag), INTEGER(Plankton.kIntegerTag),
    ARRAY(Plankton.kArrayTag), SEED(Plankton.kSeedTag);

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
