package org.javatrino.ast;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import org.neutrino.pib.CodeBundle;
import org.neutrino.pib.Module;
import org.neutrino.plankton.Store;
import org.neutrino.runtime.lookup.DigestedSignature;
import org.neutrino.syntax.Annotation;

public class Method {

  public @Store List<Annotation> annotations;
  public @Store List<Pattern> signature;
  public @Store boolean acceptsExtraArguments;
  private DigestedSignature digest;

  public @Store Expression body;
  public @Store Map<Symbol, Expression> rewrites;
  public @Store Module module;
  public @Store String fileName;

  private CodeBundle bundle;

  public Method(List<Annotation> annotations, List<Pattern> signature,
      boolean acceptsExtraArguments, Expression body, Map<Symbol, Expression> rewrites,
      Module module, String fileName) {
    assert module != null;
    this.annotations = (annotations == null) ? Annotation.NONE : annotations;
    this.signature = signature;
    this.acceptsExtraArguments = acceptsExtraArguments;
    this.body = body;
    this.rewrites = rewrites;
    this.module = module;
    this.fileName = fileName;
  }

  public Method() { }

  public DigestedSignature getDigestedSignature() {
    if (digest == null)
      digest = DigestedSignature.create(signature);
    return digest;
  }

  @Override
  public String toString() {
    return "method(" + signature.toString() + " -> " + body + ")";
  }

  public Method withRewrites(Map<Symbol, Expression> map) {
    if (map.isEmpty()) {
      return this;
    } else if (this.rewrites == null || this.rewrites.isEmpty()) {
      return new Method(annotations, signature, acceptsExtraArguments, body, map,
          module, fileName);
    } else {
      assert false;
      return null;
    }
  }

  public boolean isApplicable(int argc) {
    return signature.size() == argc;
  }

  public CodeBundle getBundle() {
    if (bundle == null) {
      List<Symbol> syms = new ArrayList<Symbol>();
      for (Pattern pattern : signature) {
        if (pattern != null)
          syms.add(pattern.symbol);
      }
      bundle = new CodeBundle(module, fileName, body, syms, rewrites);
    }
    return bundle;
  }

}
