# Standard Macro Library Checkpoint 09

## Family
- RAM

## Scope
- Admit the first fixed-name standard-library RAM:
  - `RAM8`

## Contract
- Fixed 8-bit data output width
- Address width is inferred from the explicit read/write address input lists
- Read address width must equal write address width
- Initialization word count must equal:
  - `2^addr_width`
- Read behavior is combinational over current committed storage contents
- Write behavior is synchronous and becomes visible only after commit
- Write-enable is active-high
- Same-cycle read-during-write returns the pre-commit stored value
- 4-state behavior follows the existing RAM descriptor semantics

## Implementation
- Added fixed-name `RAM8` parser support
- Kept the trusted existing RAM descriptor/runtime backend unchanged
- `RAM8` lowers through the same compiled RAM path as generic `RAM`
- No new hot-path engine behavior was required for this admission

## Validation
- [lxs_test.exe](/c:/DEV/LXS/build/bin/lxs_test.exe): passed
- `lxs_compare`:
  - `RAM8` reference vs explicit: equivalent across `128` trials

## Fixture Benchmarks
- `RAM8`
  - reference: `0.000000` GEPS
  - explicit: `0.000000` GEPS

## Decision
- Admit `RAM8` as the first standard-library RAM

## Notes
- Qualification uses:
  - [ram8_reference.bench](/c:/DEV/LXS/Tests/Circuits/ram8_reference.bench)
  - [ram8_explicit.bench](/c:/DEV/LXS/Tests/Circuits/ram8_explicit.bench)
- Like the earlier memory admissions, fixture GEPS may read as zero because RAM descriptor activity
  does not contribute combinational gate-eval work in the benchmark metric
- This is a correctness/usability admission first
