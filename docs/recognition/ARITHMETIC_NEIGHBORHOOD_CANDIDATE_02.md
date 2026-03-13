ARITHMETIC NEIGHBORHOOD CANDIDATE 02

Purpose
This document defines the next arithmetic neighborhood candidate after
CARRY_SAVE_ROW4_NEIGHBORHOOD.

The goal is to move one level up from a single local reduction row into a
bounded multiplier block that combines:
- local carry-save reduction
- immediate bounded carry propagation

Candidate Name
ROW_PAIR_REDUCE_PROPAGATE4

Class
- arithmetic
- structural-combinational
- multi-output neighborhood kernel

Semantic Contract
Inputs
- two adjacent local arithmetic rows
- four consecutive column windows per row
- fixed local carry handoff at neighborhood entry

Outputs
- four propagated low-column outputs
- bounded carry-out outputs for the next neighborhood boundary
- any remaining local reduction outputs required by the closed contract

Behavior
- exact bounded arithmetic equivalence for the chosen two-row local multiplier
  subregion
- deterministic
- zero-delay combinational
- no internal state

Concept
This candidate is not another wider ripple adder.

It is a bounded two-row block that:
- first compresses local row terms with carry-save logic
- then performs a bounded carry-propagate finish inside the same neighborhood
- exports only the neighborhood boundary results

The intent is to erase more stitched macro boundaries than
CARRY_SAVE_ROW4_NEIGHBORHOOD while still avoiding an unbounded multiplier or
adder-tree search.

Expected Structural Motif
A local multiplier neighborhood containing:
- two adjacent arithmetic rows
- four consecutive columns
- enough local term density to justify carry-save compression
- a clean local handoff point where the remaining propagated outputs can be
  emitted without leaking internal fanout

The neighborhood must remain rooted at the emitted low-order outputs and the
bounded carry boundary.

Expected Benefit Hypothesis
- beats the isolated carry-save row by amortizing its boundary cost
- absorbs more local structure than the current stitched slice2 path
- reduces the number of row-to-row and slice-to-slice handoffs in the
  multiplier rewrite
- is more likely than a wider serial ripple block to outperform the current
  packed arithmetic path

Why This Candidate Is Worth Testing
- CARRY_SAVE_ROW4 proved that larger local reduction neighborhoods can be
  profitable on EPFL multiplier
- its isolated loss suggests the missing win is at the next boundary, not in
  the local reduction semantics themselves
- the current packed slice2 path is still the arithmetic winner, so the next
  candidate must absorb more surrounding structure rather than just replacing a
  local primitive cluster

Why Existing Kernels Are Insufficient
- HALF_ADDER and FULL_ADDER are too local
- RIPPLE_SLICE2 still leaves many row and carry handoffs in multiplier
  structure
- RIPPLE_ADD4 favors serial carry propagation instead of local reduction
- CARRY_SAVE_ROW4 reduces local work but still leaves the next propagation
  boundary outside the kernel

Recognition Status
Not for generic recognition.

This candidate is for:
- explicit kernel-first implementation
- dedicated tests
- controlled offline substitution on multiplier structure

Legality Boundary
- fixed root boundary at the neighborhood outputs
- fixed row count: 2
- fixed column span: 4
- fixed local carry entry and carry exit points
- no leaking internal fanout from absorbed reduction or propagate nets
- no overlap with adjacent row-pair windows in the same pass
- strict abort if:
  - local density is insufficient
  - row ownership is ambiguous
  - internal carry nets escape the neighborhood
  - neighborhood closure cannot be expressed exactly

Negative Space
Must not match:
- single-row-only reduction groups
- windows larger than four columns
- structures requiring reassociation across more than two adjacent rows
- neighborhoods whose internal carries or partial sums are consumed outside the
  closed boundary
- global multiplier tree regions

Pilot Evaluation Order
1. explicit kernel
2. dedicated unit tests
3. controlled offline rewrite on EPFL multiplier
4. isolated benchmark delta
5. comparison against current packed slice2 multiplier path
6. admit, reject, or defer

Primary Pilot Target
- EPFL multiplier

Secondary Reference
- c6288 only if needed as a historical arithmetic check

Success Criteria
- equivalence proven under controlled rewrite
- positive isolated benchmark delta against the original multiplier
- improvement over the current carry-save row neighborhood
- credible competition with the current packed slice2 arithmetic path
- bounded telemetry remains clean

Stopping Conditions
- neighborhood requires widening search radius
- local closure cannot be defined cleanly
- runtime or scheduler instability appears
- profitability does not exceed the current carry-save row neighborhood
- profitability remains clearly below the current packed slice2 path with no
  structural reason to continue

Current Decision
Candidate accepted for kernel-first implementation planning.
