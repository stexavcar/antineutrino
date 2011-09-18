package org.neutrino.pib;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.javatrino.ast.Method;
import org.neutrino.compiler.Source;
import org.neutrino.plankton.Store;
import org.neutrino.runtime.Interpreter;
import org.neutrino.runtime.Native;
import org.neutrino.runtime.RFunction;
import org.neutrino.runtime.RModule;
import org.neutrino.runtime.RNull;
import org.neutrino.runtime.RProtocol;
import org.neutrino.runtime.RString;
import org.neutrino.runtime.RValue;
import org.neutrino.syntax.Annotation;
import org.neutrino.syntax.Tree;

public class Module {

  public Map<RValue, Binding> defs;

  public @Store RModule wrapper;
  public @Store List<Method> methods;
  public @Store Universe universe;
  public @Store Map<RProtocol, List<RProtocol>> inheritance = new HashMap<RProtocol, List<RProtocol>>();

  public @Store Map<RValue, List<Annotation>> globalAnnots;
  public @Store Map<RValue, RValue> globals;

  private Module(Universe universe, Map<RValue, Binding> defs, List<Method> methods,
      Map<RProtocol, List<RProtocol>> inheritance,
      Map<RValue, List<Annotation>> globalAnnots, Map<RValue, RValue> globals,
      RModule wrapper) {
    this.defs = defs;
    this.methods = methods;
    this.universe = universe;
    this.inheritance = inheritance;
    this.globalAnnots = globalAnnots;
    this.globals = globals;
    this.wrapper = wrapper;
    this.wrapper.setOwner(this);
  }

  public Module() { }

  public Universe getUniverse() {
    return universe;
  }

  public static Module newEmpty(Universe universe, RModule wrapper) {
    return new Module(universe,
        new HashMap<RValue, Binding>(),
        new ArrayList<Method>(),
        new HashMap<RProtocol, List<RProtocol>>(),
        new HashMap<RValue, List<Annotation>>(),
        new HashMap<RValue, RValue>(),
        wrapper);
  }

  public RModule getWrapper() {
    return wrapper;
  }

  public RValue getEntryPoint(RValue name) {
    for (Map.Entry<RValue, List<Annotation>> entry : globalAnnots.entrySet()) {
      for (Annotation annot : entry.getValue()) {
        if (annot.tag.equals("entry_point") && annot.args.get(0).equals(name)) {
          return globals.get(entry.getKey());
        }
      }
    }
    return null;
  }

  @Override
  public String toString() {
    return "a Module {globals: " + globals + ", methods: " + methods + "}";
  }

  public RValue lookupLocal(RValue name) {
    return globals.get(name);
  }

  public RValue ensureGlobal(RValue name) {
    if (!globals.containsKey(name)) {
      Binding binding = defs.get(name);
      if (binding == null)
        return null;
      globalAnnots.put(name, binding.annots);
      globals.put(name, RNull.getInstance());
      RValue result = Interpreter.run(binding.getCode());
      globals.put(name, result);
    }
    return globals.get(name);
  }

  private boolean hasEvaluatedStatics() {
    return defs == null;
  }

  public void evaluateStatics() {
    for (RValue name : defs.keySet())
      this.ensureGlobal(name);
    defs = null;
  }

  public RValue getGlobal(RValue name) {
    return getGlobal(name, false);
  }

  private RValue getGlobal(RValue name, boolean strict) {
    RValue localValue;
    if (hasEvaluatedStatics()) {
      localValue = globals.get(name);
    } else {
      localValue = ensureGlobal(name);
    }
    if (localValue != null) {
      return localValue;
    }
    List<RModule> delegates = wrapper.delegates;
    if (delegates.isEmpty() && !strict) {
      return universe.getGlobal(name);
    } else {
      for (RModule delegate : delegates) {
        RValue delegateValue = delegate.owner.getGlobal(name, true);
        if (delegateValue != null) {
          return delegateValue;
        }
      }
    }
    return null;
  }

  public void addParents(List<RProtocol> out, RProtocol id) {
    List<RProtocol> parents = inheritance.get(id);
    if (parents != null) {
      for (RProtocol parent : parents)
        out.add(parent);
    }
  }

  public void declareInheritance(RProtocol sub, RProtocol shuper) {
    List<RProtocol> locals = inheritance.get(sub);
    if (locals == null) {
      locals = new ArrayList<RProtocol>();
      inheritance.put(sub, locals);
    }
    locals.add(shuper);
  }

  public void createDefinition(RValue name, Binding binding) {
    defs.put(name, binding);
  }

  public void createMethod(Method method) {
    methods.add(method);
  }

  public RFunction getOrCreateFunction(Source origin, List<Tree.Annotation> annots,
      RValue name) {
    if (!globals.containsKey(name)) {
      RFunction result = new RFunction();
      globals.put(name, result);
      globalAnnots.put(name, processAnnotations(annots));
    }
    return (RFunction) globals.get(name);
  }

  public RProtocol createProtocol(Source origin, List<Tree.Annotation> annots,
      RValue id, RValue displayName) {
    RProtocol proto = null;
    for (Tree.Annotation annot : annots) {
      if (annot.getTag().equals(Native.ANNOTATION)) {
        proto = RProtocol.getCanonical(((RString) annot.getArguments().get(0).getValue(this)).getValue());
      }
    }
    if (proto == null)
      proto = new RProtocol(processAnnotations(annots), id, displayName);
    globals.put(id, proto);
    globalAnnots.put(id, Collections.<Annotation>emptyList());
    return proto;
  }

  private List<Annotation> processAnnotations(List<Tree.Annotation> annots) {
    List<Annotation> newAnnots;
    if (annots.isEmpty()) {
      newAnnots = Collections.emptyList();
    } else {
      newAnnots = new ArrayList<Annotation>();
      for (Tree.Annotation annot : annots)
        newAnnots.add(new Annotation(annot.getTag(), null));
    }
    return newAnnots;
  }

  public List<Annotation> getAnnotations(Object id) {
    return globalAnnots.get(id);
  }

}
