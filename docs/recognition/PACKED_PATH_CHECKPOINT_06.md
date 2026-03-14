PACKED PATH CHECKPOINT 06

Purpose
This checkpoint records the first compiler-side packed-path scheduling pass,
targeting artificial level penalties on arithmetic source multi macros.

Objective
1. inspect compiler level assignment for packed arithmetic macros
2. test whether `RIPPLE_ADD4` is being penalized artificially relative to
   chained `RIPPLE_SLICE2`
3. run multiple candidate fixes in one pass
4. keep only the candidate that improves same-batch throughput

Why this pass was attempted
- checkpoint 05 showed that reducing `RIPPLE_SLICE2` count in the generated
  artifact was not enough by itself
- the failing `RIPPLE_ADD4` merge candidate improved macro count but worsened
  `level_count`
- that suggested the next bottleneck was in compiler level modeling, not in the
  generator

Observed compiler-side issue
- source multi macros were being assigned levels from their internal primitive
  gate depth
- this likely overstated dependency layering for wider arithmetic macros,
  especially `RIPPLE_ADD4`
- the likely symptom was:
  - wider arithmetic windows looking worse than chained `RIPPLE_SLICE2`
  - even when the runtime kernel and emitted artifact should have benefited

Candidate fixes

Candidate 1
- arithmetic-only unit-level policy
- affected source multi macro types:
  - `HALF_ADDER`
  - `FULL_ADDER`
  - `RIPPLE_SLICE2`
  - `RIPPLE_ADD4`
  - `CARRY_SAVE_ROW4`
  - `REDUCE_PROPAGATE4`
- macro level is computed as:
  - `max(external input driver level) + 1`
- internal primitive depth is ignored for those arithmetic source multi macros

Candidate 2
- universal unit-level policy
- all source multi macros use:
  - `max(external input driver level) + 1`

Correctness
- rebuilt bench and test binaries from source for:
  - protected baseline compiler
  - Candidate 1
  - Candidate 2
- full test suite:
  - passed on the current source tree

Same-batch comparison

Packed adder
- protected baseline:
  - `364,814,356.510409` GEPS
  - `level_count`: `258`
- Candidate 1:
  - `522,842,745.211675` GEPS
  - `level_count`: `257`
- Candidate 2:
  - `350,917,916.832638` GEPS
  - `level_count`: `257`

Packed multiplier
- protected baseline:
  - `688,278,600.470893` GEPS
  - `level_count`: `562`
- Candidate 1:
  - `709,491,180.675140` GEPS
  - `level_count`: `436`
- Candidate 2:
  - `578,776,527.888411` GEPS
  - `level_count`: `436`

Interpretation
- Candidate 1 is the first packed-path pass that improved throughput by changing
  compiler scheduling behavior rather than generator shape
- the packed multiplier gain correlates with a large `level_count` reduction:
  - `562 -> 436`
- the emitted packed artifact stayed unchanged
- this confirms the previous suspicion:
  - `RIPPLE_ADD4` and related arithmetic source multi macros were being
    penalized artificially by internal-depth level assignment
- Candidate 2 proved the policy must stay narrow:
  - applying unit-level scheduling to every source multi macro erased the gain
  - broad unit-level modeling is too aggressive

Decision
- KEEP CANDIDATE 1
- REJECT CANDIDATE 2

Net result
- packed arithmetic scheduling improved without changing the packed artifact
- the winning fix is compiler-side and narrow
- the current packed-path direction remains validated

Next sensible move
- keep packed-path work focused on execution-cost signals that correlate with:
  - level count
  - dependency layering
  - arithmetic macro scheduling
- avoid broad compiler scheduling rewrites
- only widen this policy if a later pass proves another macro family is being
  penalized for the same reason
