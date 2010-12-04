package org.neutrino.oldton;

/**
 * A typed value which can be "grown" into a full java object based
 * on the values's type tag.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
public interface PSeed extends PValue {

  /**
   * Returns the type tag of this seed.
   */
  public PValue getTag();

  /**
   * Returns the payload object of this seed.
   */
  public PValue getPayload();

  /**
   * Grows and returns this seed as an instance of the given class.
   * If the grown instance if not an instance of the given class then
   * null is returned.
   */
  public <T> T grow(Class<T> klass);

}
