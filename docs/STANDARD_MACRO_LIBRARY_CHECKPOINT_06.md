# Standard Macro Library Checkpoint 06

## Family
- Comparator

## Scope
- Admit fixed-name standard-library comparators:
  - `CMP8`
  - `CMP16`
  - `CMP32`
  - `CMP64`

## Contract
- Combinational unsigned compare behavior
- Inputs:
  - `a[W]`
  - `b[W]`
- Outputs, in fixed order:
  - `eq`
  - `lt`
  - `gt`
- The first admitted family is unsigned only
- 4-state behavior follows normal engine propagation

## Implementation
- Added dedicated standard-comparator source/plan descriptors
- Added compiler parsing and lowering for `CMP8/16/32/64`
- Added engine execution as a single standard-library comparator step
- Added bench/profile accounting so standard comparators appear as absorbed macro work

## Validation
- [lxs_test.exe](/c:/DEV/LXS/build/bin/lxs_test.exe): passed
- `lxs_compare`:
  - `CMP8` reference vs explicit: equivalent across `128` trials
  - `CMP16` reference vs explicit: equivalent across `128` trials

## Fixture Benchmarks
- `CMP8`
  - reference: `194,516,046.622227` GEPS
  - explicit: `321,067,837.100435` GEPS
- `CMP16`
  - reference: `272,727,300.663104` GEPS
  - explicit: `886,773,486.096743` GEPS

## Structural Sanity
- `cmp8_explicit --struct-profile`
  - `comb_gates = 0`
  - `macro_steps = 1`
  - `total_gate_equiv = 89`
  - `mixed_levels = 0`

## Decision
- Admit the standard-library comparator family

## Notes
- Qualification used explicit/reference fixtures for `CMP8` and `CMP16`
- `CMP32` and `CMP64` use the same width-parameterized compiler/runtime path
- The parser-backed comparator unit test now uses stable 2-state randomized stimuli
  while `lxs_compare` remains the broader explicit/reference equivalence proof
