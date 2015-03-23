# The easy way #

Ensure that you have ant and junit installed.  Check out the source code.  Run

```
ant test
```

This will compile the java code, compile the neutrino libraries and tests into a PIB (`.pib`) file and run the tests.  The tests are quite chatty because that's currently the only way to ensure that they are actually all run.

# The flexible way #

Compile neutrino somehow, possibly using the ant script.  Compile the neutrino libraries and tests into a PIB,

```
./scripts/compile.sh --classpath=<classes> --root-path lib --outfile <pibfile>
```

You can then inspect the PIB by doing

```
./scripts/dump.sh <pibfile>
```

Add the `--raw` option to see the raw contents, otherwise you will see the fully grown object.  Finally, you can run a PIB using

```
./scripts/run.sh --classpath=<classes> --pib-path <pibfile> --entry-point <name>
```

The entry point for running the tests is `test`.

To profile any of these steps add the `--hprof` option to the command.