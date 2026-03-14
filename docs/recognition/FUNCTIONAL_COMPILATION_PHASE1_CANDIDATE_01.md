FUNCTIONAL COMPILATION PHASE 1 CANDIDATE 01

Purpose
This document defines the first arithmetic compiled-region pilot for Functional
Compilation Phase 1.

Candidate Name
ROWPAIR_REDUCE_PROPAGATE4_REGION

Class
- arithmetic
- structural-combinational
- bounded compiled region
- multi-output
- microprogram-first

Why This Candidate Exists
Phase 0 proved that bounded compiled regions can become a useful execution
layer, and the cache-aware chain-local prototype is now the preferred Phase 0
baseline.

Phase 1 now shifts that machinery into arithmetic, where the strongest
benchmark-level payoff evidence already exists.

The best starting point is not an isolated adder.
The best starting point is also not a whole multiplier or open-ended carry
tree.

The best starting point is the bounded multiplier-local structure that already
showed the strongest neighborhood evidence:
- two adjacent local arithmetic rows
- four consecutive columns
- local reduction followed by local bounded propagation

This candidate intentionally reuses that shape, but moves it into the new
compiled-region model instead of another dedicated neighborhood kernel.

Design Intent
ROWPAIR_REDUCE_PROPAGATE4_REGION should:
- absorb a local two-row four-column multiplier window
- perform both local compression and local bounded propagation internally
- export only the window boundary outputs
- execute as one fixed compiled arithmetic region object

The point is not to rediscover arithmetic with wider search.
The point is to erase more internal traffic and handoff overhead inside a
strictly bounded arithmetic window.

Semantic Contract
Inputs
- fixed local arithmetic row-pair inputs for a four-column window
- fixed carry or row-boundary entry inputs required by the local closure

Outputs
- bounded low-column finalized outputs
- bounded carry or spill outputs required for the next window boundary

Behavior
- exact deterministic combinational equivalence to the absorbed primitive
  arithmetic window
- zero-delay combinational semantics
- no internal state
- no hidden sequential interaction

Expected Structural Motif
- two adjacent arithmetic rows
- four consecutive columns
- local three-input reductions where needed
- bounded local carry propagation
- no global reassociation
- no open-ended carry continuation

Compiled-Region Requirements
- explicit multi-output functional region representation
- stable boundary input ordering
- stable boundary output ordering
- fixed microprogram schedule
- no runtime allocation
- no dynamic interpretation beyond the precompiled op list

Why This Candidate Is Worth Testing
- CARRY_SAVE_ROW4_NEIGHBORHOOD was profitable versus the original multiplier
- ROW_PAIR_REDUCE_PROPAGATE4 was also profitable versus the original
  multiplier
- both lost to the current packed slice2 path
- the likely missing improvement is not another arithmetic identity
- the likely missing improvement is less internal handoff and less temporary
  traffic inside the same bounded multiplier window

That is exactly what the compiled-region model is supposed to address.

Legality Boundary
- rooted at the emitted low-order outputs and bounded carry boundary
- fixed row count: 2
- fixed column span: 4
- fixed maximum node budget for the first pilot implementation
- fixed maximum boundary input budget
- strict abort on:
  - internal fanout leakage
  - overlap ambiguity
  - broken local closure
  - unstable carry entry or exit ownership
  - row ownership ambiguity

Negative Space
Must not match:
- single-row arithmetic windows
- wider-than-four-column windows
- open-ended ripple or propagate chains
- regions whose carries or partial sums escape outside the closed boundary
- whole multiplier trees
- any structure that requires widening search radius to complete

Execution Strategy
- microprogram-first only
- no expression execution
- no recognition attempt until kernel-first qualification is complete

Qualification Order
1. explicit compiled-region representation
2. dedicated parser-backed arithmetic fixture
3. controlled offline substitution on EPFL multiplier
4. isolated benchmark qualification
5. comparison against:
  - original multiplier
  - neighborhood mode
  - neighborhood2 mode
  - current packed slice2 path
6. checkpoint and decision

Decision Targets
- ADMIT AS ALTERNATE REGION
- ADMIT AS PREFERRED REGION
- HOLD FOR LATER
- REJECT

Current Status
Accepted for explicit compiled-region implementation planning.
