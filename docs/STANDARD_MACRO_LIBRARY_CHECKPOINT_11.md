# Standard Macro Library Checkpoint 11

## Family
- Register File

## Scope
- Admit the first standard-library register-file family:
  - `REGFILE8_2R1W`
  - `REGFILE16_2R1W`
  - `REGFILE32_2R1W`

## Contract
- Two read ports and one write port
- Outputs are ordered:
  - `read_a[W]`
  - `read_b[W]`
- Data width is fixed by family name
- Depth is inferred from the explicit address input lists
- Read-a, read-b, and write address widths must match
- Read behavior is combinational over current committed storage contents
- Write behavior is synchronous and becomes visible only after commit
- Write-enable is active-high
- Same-cycle read-during-write returns the pre-commit stored value on both read ports
- 4-state behavior follows the admitted memory-family conventions

## Implementation
- Added dedicated compiled register-file descriptor/runtime support
- Added fixed-name parser support for:
  - `REGFILE8_2R1W`
  - `REGFILE16_2R1W`
  - `REGFILE32_2R1W`
- Used duplicated generic `RAM` references only for validation, not runtime execution

## Validation
- [lxs_test.exe](/c:/DEV/LXS/build/bin/lxs_test.exe): passed
- `lxs_compare`:
  - `REGFILE8_2R1W` reference vs explicit: equivalent across `128` trials
  - `REGFILE16_2R1W` reference vs explicit: equivalent across `128` trials
  - `REGFILE32_2R1W` reference vs explicit: equivalent across `128` trials

## Fixture Benchmarks
- `REGFILE8_2R1W`
  - reference: `0.000000` GEPS
  - explicit: `0.000000` GEPS
- `REGFILE16_2R1W`
  - reference: `0.000000` GEPS
  - explicit: `0.000000` GEPS
- `REGFILE32_2R1W`
  - reference: `0.000000` GEPS
  - explicit: `0.000000` GEPS

## Decision
- Admit `REGFILE8_2R1W`, `REGFILE16_2R1W`, and `REGFILE32_2R1W`

## Notes
- Qualification uses:
  - [regfile8_reference.bench](/c:/DEV/LXS/Tests/Circuits/regfile8_reference.bench)
  - [regfile8_explicit.bench](/c:/DEV/LXS/Tests/Circuits/regfile8_explicit.bench)
  - [regfile16_reference.bench](/c:/DEV/LXS/Tests/Circuits/regfile16_reference.bench)
  - [regfile16_explicit.bench](/c:/DEV/LXS/Tests/Circuits/regfile16_explicit.bench)
  - [regfile32_reference.bench](/c:/DEV/LXS/Tests/Circuits/regfile32_reference.bench)
  - [regfile32_explicit.bench](/c:/DEV/LXS/Tests/Circuits/regfile32_explicit.bench)
- Like the earlier storage admissions, fixture GEPS reads as zero because descriptor-backed state
  access does not contribute combinational gate-eval work in the benchmark metric
