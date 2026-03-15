# Standard Macro Library Checkpoint 08

## Family
- ROM

## Scope
- Admit the first fixed-name standard-library ROM:
  - `ROM16`

## Contract
- Fixed 16-bit data output width
- Address width is inferred from the explicit address input list
- Initialization word count must equal:
  - `2^addr_width`
- Read behavior is combinational over the compiled ROM image
- ROM contents are fixed at compile/load time
- ROM does not participate in writeback or commit machinery
- 4-state behavior follows the existing ROM descriptor semantics

## Implementation
- Added fixed-name `ROM16` parser support
- Kept the trusted existing ROM descriptor/runtime backend unchanged
- `ROM16` lowers through the same compiled ROM path as generic `ROM`
- No hot-path engine behavior change was required for this admission

## Validation
- [lxs_test.exe](/c:/DEV/LXS/build/bin/lxs_test.exe): passed
- `lxs_compare`:
  - `ROM16` reference vs explicit: equivalent across `128` trials

## Fixture Benchmarks
- `ROM16`
  - reference: `0.000000` GEPS
  - explicit: `0.000000` GEPS

## Decision
- Admit `ROM16` as the first standard-library ROM

## Notes
- Qualification used:
  - [rom16_reference.bench](/c:/DEV/LXS/Tests/Circuits/rom16_reference.bench)
  - [rom16_explicit.bench](/c:/DEV/LXS/Tests/Circuits/rom16_explicit.bench)
- The zero-GEPS fixture result is expected here because the benchmark reports gate-eval throughput,
  and pure ROM descriptor lookup does not contribute combinational gate-eval work
- This is a correctness/usability admission first
