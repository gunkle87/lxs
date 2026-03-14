ENGINE HOT-PATH PHASE PLAN

Purpose
This phase targets runtime hot-path cost on top of the kept packed arithmetic
baseline.

Why This Phase Exists
- packed arithmetic is the current winning arithmetic lane
- packed-path optimization produced real wins in compiler scheduling and net
  locality
- the next plausible leverage point is the engine hot path itself, especially
  arithmetic multi-macro execution

Phase Goal
Improve end-to-end throughput by reducing engine-side arithmetic execution cost
without changing packed artifacts, weakening correctness, or hiding benchmark
cost.

Initial Hypothesis
The remaining headroom is most likely in:
- arithmetic multi-macro load/store behavior
- branch behavior inside `lxs_execute_multi_macros`
- repeated input-rail fetches in the arithmetic hot loop

Not in:
- more recognizers
- more alternate arithmetic regions
- more generator reshaping first

Scope
This phase focuses on the engine-side execution of packed arithmetic-heavy
artifacts, especially:
- [adder_macro_packed.bench](/c:/DEV/LXS/Benchmarks/Generated/adder_macro_packed.bench)
- [multiplier_macro_packed.bench](/c:/DEV/LXS/Benchmarks/Generated/multiplier_macro_packed.bench)
- [c6288_macro_packed.bench](/c:/DEV/LXS/Benchmarks/Generated/c6288_macro_packed.bench)

Method
1. pin a fresh current-head baseline
2. identify one engine-side bottleneck class
3. test multiple engine candidates in one pass
4. keep only a suite-level winner
5. reject any change that only wins on showcase artifacts

Admission Standard
- candidate must be correct
- candidate must improve the packed path honestly
- candidate must survive default-suite validation, not just artifact-level wins

Stopping Condition
If the best engine-side candidate does not survive suite-level validation, close
the phase and do not keep speculative hot-loop changes in source.
