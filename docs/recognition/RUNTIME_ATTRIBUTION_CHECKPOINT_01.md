RUNTIME ATTRIBUTION CHECKPOINT 01

Purpose
This checkpoint opens the runtime-attribution phase.

Starting Baseline
- protected commit before this phase:
  - `739f891`
- active baseline lane:
  - packed-path winner set from the prior packed-path phase
- engine hot-path phase outcome:
  - no new default runtime change admitted

Why this checkpoint exists
- the prior phase showed that arithmetic input caching alone is too weak at
  suite scale
- the next pass must attribute broader runtime cost before changing code

Opening thesis
The next suite-level gains are more likely to come from:
- primitive-boundary execution cost
- level/phase transition overhead
- mixed arithmetic/primitive level behavior

Not from:
- another small arithmetic-kernel tweak in isolation

Immediate next move
1. add runtime-attribution measurement for:
   - arithmetic-heavy levels
   - primitive-boundary levels
   - chunk-to-macro and macro-to-chunk transition counts
2. run those measurements on:
   - [adder_macro_packed.bench](/c:/DEV/LXS/Benchmarks/Generated/adder_macro_packed.bench)
   - [multiplier_macro_packed.bench](/c:/DEV/LXS/Benchmarks/Generated/multiplier_macro_packed.bench)
   - [c6288_macro_packed.bench](/c:/DEV/LXS/Benchmarks/Generated/c6288_macro_packed.bench)
3. identify one runtime bottleneck class worth a bounded change

Decision state
- phase is open
- no code change admitted yet
