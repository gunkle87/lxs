# net2bench

`net2bench` converts restricted BLIF and flat structural Verilog netlists into strict generic BENCH.

It is designed for deterministic offline conversion. The accepted input surface is intentionally narrow. `net2bench` is a converter, not a synthesis tool.

## Supported Conversions

Inputs:

- BLIF: restricted combinational subset
- Verilog: flat structural subset

Output:

- generic BENCH

## Non-Goals

`net2bench` intentionally does not support:

- behavioral Verilog
- sequential logic
- hierarchy
- module instantiations
- BLIF `.subckt`
- BLIF `.gate`
- synthesis or optimization

If a source file falls outside the supported subset, the tool should fail instead of guessing.

## Build

Requirements:

- `gcc`
- `make`
- PowerShell for the corpus test runner

Build:

```text
make
```

Output:

```text
net2bench.exe
```

## Usage

Convert BLIF to BENCH:

```text
net2bench --input circuit.blif --output circuit.bench --from blif
```

Convert structural Verilog to BENCH:

```text
net2bench --input circuit.v --output circuit.bench --from verilog
```

Lower wide gates into deterministic binary chains:

```text
net2bench --input circuit.v --output circuit.bench --from verilog --wide-gates lower-binary
```

## CLI Reference

Full CLI details are in:

- [`docs/CLI.md`](/c:/DEV/LXS/blif2bench/docs/CLI.md)

Additional phase-1 references:

- [`docs/CONTRACT.md`](/c:/DEV/LXS/blif2bench/docs/CONTRACT.md)
- [`docs/BENCH_DIALECT.md`](/c:/DEV/LXS/blif2bench/docs/BENCH_DIALECT.md)

## Determinism

`net2bench` is deterministic. Identical input plus identical flags should produce byte-identical BENCH output.

## Testing

Run the corpus tests with:

```text
make test
```

The corpus is driven by:

- [`tests/corpus/manifest.csv`](/c:/DEV/LXS/blif2bench/tests/corpus/manifest.csv)

## Repository Layout

- `src/` implementation
- `docs/` contracts and format notes
- `tests/` corpus inputs and golden outputs
