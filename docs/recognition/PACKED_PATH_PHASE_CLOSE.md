PACKED PATH PHASE CLOSE

Purpose
This document closes the packed-path optimization phase.

Why the phase existed
- packed arithmetic was already the strongest arithmetic lane
- improving the winner was higher leverage than continuing to tune alternate
  region paths

What this phase proved
- packed-path progress came from real compiler/runtime behavior, not from more
  recognition work
- the strongest gains correlated with:
  - arithmetic scheduling correctness
  - level modeling
  - net locality
- not all structural compression or locality improvements translated into
  throughput gains

Kept winners from this phase

Checkpoint 01
- adder packed rewrite now prefers `RIPPLE_ADD4` windows before falling back to
  `RIPPLE_SLICE2`
- result:
  - packed adder materially improved

Checkpoint 02
- multiplier packed rewrite now writes the final accumulation row directly to
  outputs where safe
- result:
  - packed multiplier improved

Checkpoint 06
- arithmetic source multi macros now use unit-level scheduling for supported
  arithmetic families
- result:
  - packed multiplier level count dropped materially
  - throughput improved without changing the generated artifact

Checkpoint 08
- thresholded arithmetic input/output net clustering was added to the compiler
- policy:
  - activate only when `source_multi_macro_count >= 128`
- result:
  - packed multiplier arithmetic input locality improved massively
  - packed adder stayed on the simple baseline path
  - both packed adder and packed multiplier improved on the admitted batch

Rejected branches from this phase

Checkpoint 03
- aggressive diagonal-output cleanup
- rejected:
  - looked better structurally
  - regressed runtime

Checkpoint 04 / 05
- generator-side `RIPPLE_SLICE2` reduction by merging into `RIPPLE_ADD4`
- rejected:
  - reduced macro count
  - worsened levelization enough to lose throughput

Checkpoint 07
- same-level arithmetic macro grouping and engine dispatch specialization
- rejected:
  - ordering candidate was invalid
  - dispatch candidate was too mixed across adder and multiplier

Checkpoint 09
- delayed arithmetic-output placement and boundary-output clustering
- rejected:
  - locality metrics improved sharply
  - stable longer-run A/B throughput regressed

Final kept packed-path baseline
- adder packed-window widening
- multiplier output staging cleanup
- arithmetic unit-level source multi scheduling
- thresholded arithmetic input/output clustering

Current reference measurements
- [adder_macro_packed.bench](/c:/DEV/LXS/Benchmarks/Generated/adder_macro_packed.bench)
  - `404,813,287.769544` GEPS
- [multiplier_macro_packed.bench](/c:/DEV/LXS/Benchmarks/Generated/multiplier_macro_packed.bench)
  - `731,809,566.821486` GEPS
- [c6288_macro_packed.bench](/c:/DEV/LXS/Benchmarks/Generated/c6288_macro_packed.bench)
  - `535,831,352.231003` GEPS

Interpretation
- the packed path is still the highest-leverage arithmetic lane
- this phase produced real kept wins
- the remaining headroom is now harder:
  - structural or locality metrics alone are no longer trustworthy
  - any new packed-path pass must be justified by stable throughput, not just
    by better-looking profiles

Decision
- CLOSE packed-path optimization as a phase
- carry the kept baseline forward
- start the next phase from the protected packed-path winner set, not from any
  rejected experimental branch
