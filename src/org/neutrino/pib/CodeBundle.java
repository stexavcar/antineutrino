package org.neutrino.pib;

import java.util.Collections;
import java.util.List;
import java.util.Map;

import org.javatrino.ast.Expression;
import org.javatrino.ast.Symbol;
import org.javatrino.bytecode.BytecodeCompiler;
import org.neutrino.plankton.Store;

public class CodeBundle {

  public @Store String fileName;
  public @Store Expression body;
  public @Store List<Symbol> params;
  public @Store Map<Symbol, Expression> rewrites;
  public @Store Module module;

  private byte[] code;
  private List<Object> literals;
  private int localCount = -1;

  public CodeBundle(byte[] code, List<Object> literals, int localCount) {
    this.code = code;
    this.literals = literals;
    this.localCount = localCount;
  }

  public CodeBundle(Module module, String fileName, Expression body, List<Symbol> params,
      Map<Symbol, Expression> rewrites) {
    assert module != null;
    this.fileName = fileName;
    this.body = body;
    this.params = params == null ? Collections.<Symbol>emptyList() : params;
    this.rewrites = rewrites;
    this.module = module;
  }

  public CodeBundle() { }

  public byte[] getCode() {
    if (code == null)
      ensureCompiled();
    return code;
  }

  public int getLocalCount() {
    if (localCount == -1)
      ensureCompiled();
    return localCount;
  }


  public List<Object> getLiterals() {
    if (literals == null)
      ensureCompiled();
    return literals;
  }

  private void ensureCompiled() {
    if (code == null) {
      BytecodeCompiler.Result result = BytecodeCompiler.compile(module, body, params,
          rewrites);
      this.code = result.code;
      this.literals = result.literals;
      this.localCount = result.localCount;
    }
  }

}
