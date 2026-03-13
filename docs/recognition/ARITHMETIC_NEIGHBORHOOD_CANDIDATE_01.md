ARITHMETIC NEIGHBORHOOD CANDIDATE 01

Purpose
This document defines the first pilot candidate for the neighborhood-kernel
phase.

The goal is to replace a bounded local arithmetic region with one execution
kernel that absorbs more intermediate structure than the current local
arithmetic macros.

Candidate Name
RIPPLE_ADD4_NEIGHBORHOOD

Class
- arithmetic
- structural-combinational
- multi-output neighborhood kernel

Semantic Contract
Inputs
- a0
- b0
- a1
- b1
- a2
- b2
- a3
- b3
- cin

Outputs
- sum0
- sum1
- sum2
- sum3
- cout

Behavior
- exact 4-bit ripple addition
- sum[3:0], cout = a[3:0] + b[3:0] + cin
- deterministic
- zero-delay combinational
- no internal state

Expected Structural Motif
A bounded 4-bit ripple-add neighborhood containing:
- four serial full-adder stages
or
- two trusted 2-bit arithmetic slices with chained carry
or
- an equivalent local full-adder chain produced by trusted offline rewrite

This candidate is intentionally rooted at the neighborhood outputs:
- sum3
- cout

The neighborhood must be fully local and structurally closed.

Expected Benefit Hypothesis
- absorbs more intermediate carry and sum structure than:
  - HALF_ADDER
  - FULL_ADDER
  - RIPPLE_SLICE2
- reduces boundary handoff cost across multiple 1-bit or 2-bit arithmetic
  fragments
- gives arithmetic the first true neighborhood-scale execution unit

Why This Candidate Is Worth Testing
- arithmetic already has the strongest trusted offline substitution wins
- arithmetic is the clearest proof that larger bounded structure can pay
- previous small arithmetic motifs were often too local to amortize macro
  boundary cost
- previous large arithmetic wins were seen in offline rewritten adder and
  multiplier forms

Why Existing Kernels Are Insufficient
- HALF_ADDER and FULL_ADDER are too local
- RIPPLE_SLICE2 improves locality but still leaves cross-slice boundary cost
- family-level recognition proved that local arithmetic kernels alone are not
  enough to produce broad recognition wins

Recognition Status
Not for generic recognition yet.

This candidate is for:
- kernel-first implementation
- controlled offline substitution
- isolated qualification

Only after that may recognition be considered.

Legality Boundary
- fixed root boundary at the 4-bit neighborhood outputs
- fixed maximum size:
  - 4 bit positions
  - one carry chain
- no leaking internal fanout from absorbed intermediate carry or local sum nets
- no overlap with larger arithmetic neighborhood in the same pass
- strict abort if:
  - carry chain is broken
  - internal outputs fan out outside the neighborhood
  - ambiguous stage ownership exists

Negative Space
Must not match:
- partial 4-bit regions with missing carry continuity
- structures where intermediate carry is consumed outside the neighborhood
- overlapping arithmetic windows sharing the same internal carry nodes
- broad global adder trees or multiplier regions

Pilot Evaluation Order
1. explicit kernel
2. dedicated unit tests
3. controlled offline rewrite on a trusted arithmetic benchmark
4. isolated benchmark delta
5. admit, reject, or defer

Primary Pilot Targets
- EPFL adder
- EPFL multiplier subregions only if needed later

Secondary Reference
- c6288 only as a historical arithmetic reference, not the primary proof point

Success Criteria
- equivalence proven under controlled rewrite
- absorbed work share higher than current local arithmetic substitution
- isolated benchmark delta clearly positive
- bounded telemetry remains clean

Stopping Conditions
- profitability does not improve over RIPPLE_SLICE2-based substitution
- legality requires widening search radius
- the neighborhood causes scheduler or overlap ambiguity
- execution cost grows faster than absorbed work

Current Decision
Candidate accepted for kernel-first implementation planning.
