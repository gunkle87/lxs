# Standard Macro Library Checkpoint 10

## Family
- RAM

## Scope
- Extend the admitted standard-library RAM family with:
  - `RAM16`
  - `RAM24`

## Contract
- Uses the existing admitted `RAM8` contract without semantic changes
- Fixed data output width is defined by family name:
  - `RAM16`
  - `RAM24`
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
- Extended fixed-name RAM parsing from `RAM8` to:
  - `RAM16`
  - `RAM24`
- Kept the trusted existing RAM descriptor/runtime backend unchanged
- No new hot-path engine behavior was required for these width extensions

## Validation
- [lxs_test.exe](/c:/DEV/LXS/build/bin/lxs_test.exe): passed
- `lxs_compare`:
  - `RAM16` reference vs explicit: equivalent across `128` trials
  - `RAM24` reference vs explicit: equivalent across `128` trials

## Fixture Benchmarks
- `RAM16`
  - reference: `0.000000` GEPS
  - explicit: `0.000000` GEPS
- `RAM24`
  - reference: `0.000000` GEPS
  - explicit: `0.000000` GEPS

## Decision
- Admit `RAM16` and `RAM24` as width extensions of the standard-library RAM family

## Notes
- Qualification uses:
  - [ram16_reference.bench](/c:/DEV/LXS/Tests/Circuits/ram16_reference.bench)
  - [ram16_explicit.bench](/c:/DEV/LXS/Tests/Circuits/ram16_explicit.bench)
  - [ram24_reference.bench](/c:/DEV/LXS/Tests/Circuits/ram24_reference.bench)
  - [ram24_explicit.bench](/c:/DEV/LXS/Tests/Circuits/ram24_explicit.bench)
- Like the earlier memory admissions, fixture GEPS may read as zero because RAM descriptor activity
  does not contribute combinational gate-eval work in the benchmark metric
- This is a correctness/usability admission first
