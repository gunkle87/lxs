BASELINE LEDGER

Purpose
This file centralizes the benchmark baselines that are most often referenced in
recognition, packed-path, and runtime-attribution work.

Reading rule
- compare only like-with-like when possible
- if a value is historical and was not freshly rerun under the current
  toolchain/settings, mark it as recorded rather than rerun
- use this file to separate:
  - recorded historical references
  - fresh same-toolchain comparisons

Ledger

1. Recognition Baseline R0
- commit: `2636584`
- phase meaning:
  - pinned pre-recognition baseline reference
- corpus:
  - default benchmark suite at the time of R0
- settings:
  - historical recorded reference
- weighted GEPS:
  - `456,660,260.470426`
- provenance:
  - recorded in [RECOGNITION_SUMMARY.md](/c:/DEV/LXS/docs/recognition/RECOGNITION_SUMMARY.md)
  - repeated in [RECOGNITION_SNAPSHOT_R1.md](/c:/DEV/LXS/docs/recognition/RECOGNITION_SNAPSHOT_R1.md)
- note:
  - use as a historical milestone reference
  - not a same-toolchain rerun comparison

2. Runtime Attribution Baseline
- commit: `11adfcc`
- phase meaning:
  - previous protected stable before arithmetic-boundary `AND` fast path
- corpus:
  - `Benchmarks\Benches`
- settings:
  - `samples=5`
  - `sample-select=median`
  - `iterations=1`
  - `cycles=10000`
- weighted GEPS:
  - `528,499,323.799671`
- provenance:
  - fresh same-toolchain rerun comparison against `5301be4`
- note:
  - this is the most rigorous immediate comparison point for current head

3. Current Protected Runtime Attribution Winner
- commit: `5301be4`
- phase meaning:
  - admitted arithmetic-boundary `AND` chunk fast path
- corpus:
  - `Benchmarks\Benches`
- settings:
  - `samples=5`
  - `sample-select=median`
  - `iterations=1`
  - `cycles=10000`
- weighted GEPS:
  - `533,895,804.508597`
- provenance:
  - fresh same-toolchain rerun comparison against `11adfcc`
- delta vs `11adfcc`:
  - `+5,396,480.708926`
  - `+1.0211%`

4. Current Head vs Recorded R0
- current commit:
  - `5301be4`
- historical reference commit:
  - `2636584`
- current weighted GEPS:
  - `533,895,804.508597`
- recorded R0 weighted GEPS:
  - `456,660,260.470426`
- delta:
  - `+77,235,544.038171`
  - `+16.91%`
- note:
  - this is a useful headline comparison
  - it is less rigorous than the same-toolchain `11adfcc` comparison

Usage guidance
- for current engineering decisions:
  - prefer `11adfcc` vs `5301be4`
- for historical project narrative:
  - use `2636584` R0 with explicit `recorded historical baseline` language
- do not mix:
  - recorded historical values
  - same-toolchain reruns
without saying so explicitly
