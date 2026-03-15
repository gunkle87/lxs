# Standard Macro Library Checkpoint 07

## Family
- ALU

## Scope
- Admit fixed-name standard-library ALUs:
  - `ALU8`
  - `ALU16`
  - `ALU32`
  - `ALU64`

## Contract
- Combinational core ALU behavior
- Inputs:
  - `a[W]`
  - `b[W]`
  - `op0`
  - `op1`
  - `op2`
- Outputs:
  - `y[W]`
  - `cout`
  - `eq`
  - `lt`
  - `gt`
- Op encoding:
  - `000` = add
  - `001` = sub
  - `010` = and
  - `011` = or
  - `100` = xor
  - `101` = pass `a`
  - `110` = pass `b`
  - `111` = zero
- `eq/lt/gt` are always the unsigned compare flags for `a` versus `b`
- `cout` is meaningful for add/sub and zero for the other ops
- 4-state behavior follows normal engine propagation

## Implementation
- Added dedicated standard-ALU source/plan descriptors
- Added compiler parsing and lowering for `ALU8/16/32/64`
- Added engine execution as a single standard-library ALU step
- Added bench/profile accounting so standard ALUs appear as absorbed macro work

## Validation
- [lxs_test.exe](/c:/DEV/LXS/build/bin/lxs_test.exe): passed
- `lxs_compare`:
  - `ALU8` reference vs explicit: equivalent across `128` trials
  - `ALU16` reference vs explicit: equivalent across `128` trials

## Fixture Benchmarks
- `ALU8`
  - reference: `450,419,073.838721` GEPS
  - explicit: `1,175,925,950.350840` GEPS
- `ALU16`
  - reference: `611,237,051.415798` GEPS
  - explicit: `948,581,574.806668` GEPS

## Structural Sanity
- `alu8_explicit --struct-profile`
  - `comb_gates = 0`
  - `macro_steps = 1`
  - `total_gate_equiv = 381`
  - `arithmetic_only_levels = 1`

## Decision
- Admit the standard-library ALU family

## Notes
- Qualification used explicit/reference fixtures for `ALU8` and `ALU16`
- `ALU32` and `ALU64` use the same width-parameterized compiler/runtime path
- The reference fixtures intentionally compose previously admitted building blocks
  (`ADD*`, `CMP*`, `MUX*`) with primitive selection logic rather than depending
  on handwritten monolithic structural netlists
