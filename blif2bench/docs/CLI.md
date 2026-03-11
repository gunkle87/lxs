# CLI

```text
net2bench --input <path> --output <path> [--from auto|blif|verilog] [--to bench] [--strict] [--validate-only] [--overwrite error|replace] [--stdout] [--quiet] [--name-map <path>] [--wide-gates preserve|lower-binary] [--constants reject]
```

Phase 1 defaults:

- `--from auto`
- `--to bench`
- `--strict`
- `--overwrite error`
- `--wide-gates preserve`
- `--constants reject`

Wide-gate behavior:

- `--wide-gates preserve` emits wide `AND`, `NAND`, `OR`, and `NOR` directly
- `--wide-gates lower-binary` lowers wide `AND`, `NAND`, `OR`, and `NOR` into deterministic binary chains using `__t_XXXXXX` helpers
