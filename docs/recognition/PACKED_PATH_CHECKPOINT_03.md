PACKED PATH CHECKPOINT 03

Purpose
This checkpoint records the third packed-path optimization pass.

Objective
1. isolate one additional concrete packed-path bottleneck
2. implement one equally narrow change
3. re-measure before doing anything broader

Baseline

Packed multiplier before this pass
- packed:
  - `685,449,090.797176` GEPS
  - `chunks`: `65`
  - `primitive_steps`: `65`
  - `macro_steps`: `2079`
  - `total_gate_equiv`: `24128`
  - `buf`: `63`

Concrete bottleneck
- the packed multiplier still buffered the finalized diagonal outputs
  `f[1]..f[62]` from nets of the form `add_k_sum_k`
- those bits are structurally final at row `k`
- this made the next narrow candidate clear:
  - emit finalized diagonal sums directly to product outputs
  - keep only the unavoidable `f[0] = BUF(pp_0_0)` boundary

Targeted optimization
- changed packed multiplier rewrite generation so finalized diagonal sums write
  directly to `f[k]` at row `k`
- kept the change narrow:
  - generator-only
  - no runtime change
  - no recognition change
  - no structural widening

Correctness
- regenerated packed multiplier artifact:
  - equivalent to original multiplier across `512` trials
- full test suite:
  - passed

After optimization
- packed:
  - `593,573,393.893120` GEPS
  - `chunks`: `3`
  - `primitive_steps`: `3`
  - `macro_steps`: `2079`
  - `total_gate_equiv`: `24066`
  - `buf`: `1`

Interpretation
- the bottleneck was real in structural terms
- the change almost completely removed trailing primitive output staging
- but the runtime result regressed materially
- this means the eliminated boundary layer was not the dominant cost in the
  actual packed execution path
- the pass is therefore not admitted as a packed-path improvement

Post-pass finding
- the strongest remaining execution-cost hypothesis is not chunk count
- the packed multiplier hot path is dominated by arithmetic dispatch shape:
  - approximately `1953` `RIPPLE_SLICE2` operations
  - versus only `64` `HALF_ADDER` and `62` `FULL_ADDER` operations
- that means the likely cost-correlated bottleneck is over-segmented row
  accumulation, not the residual primitive output layer
- a future packed-path pass should therefore target arithmetic window
  granularity in the row accumulator, and only if that can be done in a way
  that actually changes the generated op mix

Decision
- REJECT CHANGE

Next sensible move
- restore the pre-pass packed multiplier generator
- do not pursue another packed-path pass unless a new bottleneck can be shown
  to correlate with runtime rather than only structural compression
