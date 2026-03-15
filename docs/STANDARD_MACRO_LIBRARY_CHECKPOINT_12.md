# Standard Macro Library Checkpoint 12

## Family
- Counter

## Scope
- Admit the first fixed-width counter family:
  - `COUNTER8`
  - `COUNTER16`
  - `COUNTER32`
  - `COUNTER64`
  - `COUNTER_EN8`
  - `COUNTER_EN16`
  - `COUNTER_EN32`
  - `COUNTER_EN64`

## Contract
- `COUNTER*` increments once per tick
- `COUNTER_EN*` increments once per tick only when enable is asserted
- Enable is active-high
- Count wraps modulo `2^W`
- Updated state becomes visible after commit
- Unknown enable follows the existing counter-control 4-state behavior

## Implementation
- Added fixed-name parser support for the admitted counter widths
- Kept the trusted existing counter runtime backend unchanged
- No new hot-path engine behavior was required for counter-family admission

## Validation
- [lxs_test.exe](/c:/DEV/LXS/build/bin/lxs_test.exe): passed
- `lxs_compare`:
  - `COUNTER8`: covered by the parser-backed constant-enable reference path in [lxs_test.c](/c:/DEV/LXS/src/apps/lxs_test.c)
  - `COUNTER16`: covered by the parser-backed constant-enable reference path in [lxs_test.c](/c:/DEV/LXS/src/apps/lxs_test.c)
  - `COUNTER_EN8` reference vs explicit: equivalent across `128` trials
  - `COUNTER_EN16` reference vs explicit: equivalent across `128` trials

## Fixture Benchmarks
- `COUNTER8`
  - reference: `0.000000` GEPS
  - explicit: `0.000000` GEPS
- `COUNTER16`
  - reference: `0.000000` GEPS
  - explicit: `0.000000` GEPS
- `COUNTER_EN8`
  - reference: `0.000000` GEPS
  - explicit: `0.000000` GEPS
- `COUNTER_EN16`
  - reference: `0.000000` GEPS
  - explicit: `0.000000` GEPS

## Decision
- Admit the fixed-width counter family through the declared widths

## Notes
- Qualification uses:
  - [counter8_reference.bench](/c:/DEV/LXS/Tests/Circuits/counter8_reference.bench)
  - [counter8_explicit.bench](/c:/DEV/LXS/Tests/Circuits/counter8_explicit.bench)
  - [counter16_reference.bench](/c:/DEV/LXS/Tests/Circuits/counter16_reference.bench)
  - [counter16_explicit.bench](/c:/DEV/LXS/Tests/Circuits/counter16_explicit.bench)
  - [counter_en8_reference.bench](/c:/DEV/LXS/Tests/Circuits/counter_en8_reference.bench)
  - [counter_en8_explicit.bench](/c:/DEV/LXS/Tests/Circuits/counter_en8_explicit.bench)
  - [counter_en16_reference.bench](/c:/DEV/LXS/Tests/Circuits/counter_en16_reference.bench)
  - [counter_en16_explicit.bench](/c:/DEV/LXS/Tests/Circuits/counter_en16_explicit.bench)
- Plain `COUNTER*` uses a dedicated constant-enable reference path in the parser-backed test suite because
  the trusted generic reference form is `COUNTER_EN(en)` with `en` held high each cycle
