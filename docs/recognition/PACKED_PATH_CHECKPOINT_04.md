PACKED PATH CHECKPOINT 04

Purpose
This checkpoint records a targeted row-accumulator granularity pass on the
packed multiplier path.

Objective
1. target a bottleneck that correlates with execution cost rather than chunk
   count
2. reduce `RIPPLE_SLICE2` dominance in the packed multiplier artifact
3. re-measure against the protected pre-pass artifact on the same command

Why this pass was attempted
- the packed multiplier hot path was dominated by row-accumulator arithmetic
  dispatch shape:
  - `1953` `RIPPLE_SLICE2`
  - `64` `HALF_ADDER`
  - `62` `FULL_ADDER`
- this was a better candidate than chunk-count cleanup because it directly
  changed arithmetic macro dispatch, not just boundary staging

Targeted change
- added a generator post-pass that merged adjacent chained `RIPPLE_SLICE2`
  pairs into `RIPPLE_ADD4` windows in the packed multiplier artifact
- kept the change narrow:
  - generator-only
  - no runtime change
  - no recognition change
  - no structural widening outside packed row accumulation

Artifact change
- pre-pass mix:
  - `1953` `RIPPLE_SLICE2`
  - `64` `HALF_ADDER`
  - `62` `FULL_ADDER`
- candidate mix:
  - `1007` `RIPPLE_ADD4`
  - `1` `RIPPLE_SLICE2`
  - `1` `HALF_ADDER`
  - `1` `FULL_ADDER`

Correctness
- regenerated candidate artifact:
  - equivalent to original EPFL multiplier across `512` trials
- full test suite:
  - passed

Same-batch comparison

Protected pre-pass artifact
- [multiplier_macro_packed.bench](/c:/DEV/LXS/Benchmarks/Generated/multiplier_macro_packed.bench) at commit `03e748f`:
  - `656,117,518.687200` GEPS
  - `macro_steps`: `2079`
  - `level_count`: `562`

Candidate artifact
- row-accumulator `RIPPLE_ADD4` merged candidate:
  - `644,193,925.129780` GEPS
  - `macro_steps`: `1134`
  - `level_count`: `810`

Interpretation
- the candidate did reduce the arithmetic dispatch count materially
- but it also increased levelization depth sharply
- throughput regressed on the same batch despite the better-looking op mix
- this means the cost-correlated issue is not raw `RIPPLE_SLICE2` count alone
- the stronger signal is:
  - arithmetic granularity must be improved without worsening scheduling depth
    or dependency layering

Decision
- REJECT CHANGE

Restored state
- the packed multiplier generator and artifact were restored to the protected
  pre-pass winning baseline after this measurement

Next sensible move
- future packed-path work should treat:
  - arithmetic op mix
  - level count
  - dependency layering
  as a coupled problem
- do not pursue another `RIPPLE_SLICE2` reduction pass unless it also preserves
  or improves levelization behavior
