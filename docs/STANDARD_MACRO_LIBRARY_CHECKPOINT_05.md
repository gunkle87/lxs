# Standard Macro Library Checkpoint 05

## Family
- Addition

## Scope
- Admit fixed-name standard-library adders:
  - `ADD8`
  - `ADD16`
  - `ADD32`
  - `ADD64`

## Contract
- Combinational ripple-add behavior
- Inputs:
  - `a[W]`
  - `b[W]`
  - `cin`
- Outputs:
  - `sum[W]`
  - `cout`
- 4-state semantics follow normal engine propagation

## Implementation
- Added dedicated standard-adder source/plan descriptors
- Added compiler parsing and lowering for `ADD8/16/32/64`
- Added engine execution as a single standard-library arithmetic step
- Added bench/profile accounting so standard adders appear as absorbed macro work

## Validation
- [lxs_test.exe](/c:/DEV/LXS/build/bin/lxs_test.exe): passed
- `lxs_compare`:
  - `ADD8` reference vs explicit: equivalent across `128` trials
  - `ADD16` reference vs explicit: equivalent across `128` trials

## Fixture Benchmarks
- `ADD8`
  - reference: `166,195,783.760190` GEPS
  - explicit: `165,084,602.768478` GEPS
- `ADD16`
  - reference: `242,416,871.793322` GEPS
  - explicit: `246,457,170.314676` GEPS

## Structural Sanity
- `add8_explicit --struct-profile`
  - `comb_gates = 0`
  - `macro_steps = 1`
  - `total_gate_equiv = 40`
  - `arithmetic_only_levels = 1`

## Decision
- Admit the standard-library addition family

## Notes
- Qualification used explicit/reference fixtures for `ADD8` and `ADD16`
- `ADD32` and `ADD64` use the same width-parameterized compiler/runtime path
- This is a correctness/usability admission first; isolated fixture throughput is acceptable but not the primary gate
