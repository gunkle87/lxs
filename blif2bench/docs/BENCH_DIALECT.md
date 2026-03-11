# BENCH Dialect

Phase 1 emits only:

- `INPUT(name)`
- `OUTPUT(name)`
- `name = BUF(arg)`
- `name = NOT(arg)`
- `name = AND(arg1,arg2,...)`
- `name = NAND(arg1,arg2,...)`
- `name = OR(arg1,arg2,...)`
- `name = NOR(arg1,arg2,...)`
- `name = XOR(arg1,arg2)`
- `name = XNOR(arg1,arg2)`

Rules:

- with `--wide-gates preserve`, wide `AND`, `NAND`, `OR`, and `NOR` are emitted directly
- with `--wide-gates lower-binary`, wide `AND`, `NAND`, `OR`, and `NOR` are lowered into deterministic binary chains using `__t_XXXXXX` helper names
- `XOR` and `XNOR` are emitted only as 2-input gates
- no constants
- no inline inversion
- no custom operators
- temporary nets use `__t_000001`, `__t_000002`, and so on
