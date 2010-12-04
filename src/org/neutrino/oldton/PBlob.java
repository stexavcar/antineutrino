package org.neutrino.oldton;

/**
 * The plankton representation of a piece of binary data.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
public interface PBlob extends PValue {

  /**
   * Returns the size of this data block.
   */
  public int length();

  /**
   * Returns the byte at the given offset.
   */
  public int get(int offset);

  /**
   * Returns the full data of this blob.
   */
  public byte[] getData();

}
