# Phase 1 Contract

`net2bench` is a standalone C executable.

It accepts:

- restricted BLIF
- flat structural Verilog

It emits:

- generic primitive BENCH only

It does not do:

- macro recognition
- engine integration
- synthesis
- hierarchy flattening
- sequential lowering

Locked rules:

- duplicate identical `.inputs` and `.outputs` declarations are tolerated and deduplicated in first-seen order
- conflicting redeclarations are hard errors
- multi-driver detection happens after alias normalization
- if multiple nodes are ready at the same dependency level, emission ties are broken by canonical output-net name ascending, then by source parse order
- `assign y = x;` normalizes as alias first
- aliased primary outputs are materialized with `BUF` when the output name must exist as a real emitted net
- `--wide-gates preserve` emits wide `AND`, `NAND`, `OR`, and `NOR` directly
- `--wide-gates lower-binary` lowers wide `AND`, `NAND`, `OR`, and `NOR` into deterministic binary chains
