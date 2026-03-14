PACKED PATH CHECKPOINT 05

Purpose
This checkpoint records a second row-accumulator granularity pass on the packed
multiplier path, using a generator post-pass that actually changed the emitted
arithmetic op mix.

Objective
1. target a bottleneck that correlates with execution cost
2. prove that the generated artifact's arithmetic dispatch mix changes
3. re-measure against the protected packed baseline on the same command

Why this pass was attempted
- checkpoint 04 showed that `RIPPLE_SLICE2` dominance was a more credible
  execution-cost signal than chunk count
- the remaining question was whether a real reduction in that op mix would pay
  off when measured honestly

Targeted change
- added a narrow generator post-pass that merged adjacent chained
  `RIPPLE_SLICE2` pairs into `RIPPLE_ADD4` windows in the packed multiplier
  artifact
- kept the change narrow:
  - generator-only
  - no runtime change
  - no recognition change
  - no structural widening beyond packed row accumulation

Artifact change
- protected baseline op mix:
  - `1953` `RIPPLE_SLICE2`
  - `64` `HALF_ADDER`
  - `62` `FULL_ADDER`
- candidate op mix:
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
- commit `03e748f` packed multiplier artifact:
  - `656,117,518.687200` GEPS
  - `macro_steps`: `2079`
  - `level_count`: `562`

Candidate artifact
- merged row-accumulator window candidate:
  - `644,193,925.129780` GEPS
  - `macro_steps`: `1134`
  - `level_count`: `810`

Interpretation
- this pass did exactly what earlier passes failed to do:
  - it materially reduced the arithmetic dispatch count in the generated
    artifact
- but it still lost on throughput in the same batch
- the reason is now clearer:
  - reducing arithmetic dispatch count alone is not sufficient
  - the candidate increased levelization depth and dependency layering enough
    to erase the dispatch savings

Conclusion
- the real packed-path bottleneck is not:
  - chunk count alone
  - primitive boundary staging alone
  - raw `RIPPLE_SLICE2` count alone
- the real constraint is coupled:
  - arithmetic granularity must improve without increasing level count

Decision
- REJECT CHANGE

Restored state
- the packed multiplier generator and artifact were restored to the protected
  packed baseline after this measurement

Next sensible move
- only continue packed-path work if the next hypothesis can preserve or improve:
  - arithmetic op mix
  - level count
  - dependency layering
- do not pursue another `RIPPLE_SLICE2` reduction pass by itself
