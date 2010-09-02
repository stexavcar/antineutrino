package org.neutrino.plankton;

/**
 * A simple finite mapping from values to values.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
public interface PMap extends PValue {

  /**
   * A callback that can be used to iterate through all elements of
   * a map.
   *
   * Death to checked exceptions.
   */
  public interface ThrowingThunk<E extends Throwable> {
    public boolean call(PValue key, PValue value) throws E;
  }

  /**
   * A simpler thunk that you can use to avoid dealing with
   * exceptions.
   */
  public interface Thunk extends ThrowingThunk<RuntimeException> { }

  /**
   * Invoke the given thunk for each element in this mapping.  Stop
   * iteration after if call to the thunk returns false.
   */
  public <E extends Throwable> void forEach(ThrowingThunk<E> thunk) throws E;

  /**
   * Returns the number of elements in this map.
   */
  public int size();

  /**
   * Returns the mapping for the given key if one exists, otherwise
   * the default value.
   */
  public PValue get(PValue key, PValue ifMissing);

}
