package org.neutrino.plankton;

/**
 * A simple finite length array.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
public interface PArray extends PValue {

  /**
   * Returns the number of elements in this array.
   */
  public int length();

  /**
   * Returns the index'th element in this array.
   */
  public PValue get(int index);

}
