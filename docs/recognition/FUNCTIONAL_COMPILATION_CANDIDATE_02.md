FUNCTIONAL COMPILATION CANDIDATE 02

Purpose
This document defines the next bounded Phase 0 candidate after
WIDE_ROOT_CONE6.

Candidate Name
CACHE_LOCAL_CONE6

Class
- structural-combinational
- single-output
- bounded generic cone
- cache-aware
- microprogram-biased

Why This Candidate Exists
GENERIC_CONE5_ROOTED proved that bounded functional compilation is real.
WIDE_ROOT_CONE6 proved that wider rooted expression structure can broaden live
coverage.

But both candidates still failed the default replacement gate.

The current evidence suggests that expression richness alone is not the missing
piece. The stronger likely problem is local execution cost:
- boundary handoff still dominates
- local temporary traffic is still too high
- direct expression execution is slower than the microprogram path
- widened coverage does not translate into replacement profitability

The next candidate should therefore target locality, temp reuse, and stable
linear evaluation order rather than a richer root expression.

Design Intent
CACHE_LOCAL_CONE6 keeps the same rooted, single-output, fail-fast legality
model, but changes the intended region shape:

- one bounded root gate
  - binary or ternary preferred
- two or three tightly local preprocess gates
- topological order chosen for temp reuse
- no memo recursion required for the preferred execution form
- no wide root required
- region should favor:
  - short live ranges
  - minimal temp fanout
  - stable input ordering
  - linear local evaluation

This candidate is meant to test whether a better local working set can outperform
both:
- the earlier stitched binary cone
- the richer wide-root cone

Why It May Fit Runtime Better
- fewer temporary values alive at once
- less boundary-to-internal handoff churn
- more sequential local access within the region
- better fit for the existing microprogram execution path
- more likely to benefit cache behavior than the expression-heavy wide-root form

Semantic Contract
- one output
- exact deterministic combinational equivalence to the absorbed primitive cone
- no internal state
- no hidden sequential interaction
- no semantic widening beyond the absorbed subgraph

Legality Boundary
- root anchored at one output gate
- root type must be one of:
  - AND
  - OR
  - XOR
  - NAND
  - NOR
  - XNOR
- max depth: 3
- max node budget: 6
- max boundary inputs: 6
- max internal fanout: 1
- output count: 1
- strict abort on:
  - fanout leakage
  - branch growth
  - overlap ambiguity
  - budget overflow
  - depth overflow
  - unstable root shape

Additional locality preference
- candidate should be rejected if:
  - temp_count exceeds 2
  - any internal value is consumed by more than one later internal op
  - local evaluation order cannot remain strictly forward

Negative Space
Must not match:
- purely arithmetic carry chains
- multi-output regions
- any cone crossing sequential boundaries
- any cone whose internal node fanout escapes the region
- any region requiring recursive recomputation to evaluate efficiently
- any region whose benefit depends primarily on a wide root fold

Expected Execution Strategy
Two allowed forms:
- fixed microprogram evaluation
  - expected preferred form for this candidate
- direct expression evaluation
  - only if the discovered shape is trivially expression-safe

Expected Benefit Hypothesis
- better replacement profitability than WIDE_ROOT_CONE6
- less sensitive to expression overhead
- better fit for cache-aware local execution
- more likely to convert nontrivial coverage into actual suite-level benefit

What This Candidate Must Prove
- locality-aware bounded cones can outperform the earlier stitched binary cone
- locality-aware bounded cones can outperform the wide-root cone in replacement
  mode
- microprogram execution can benefit from shape discipline, not just larger
  absorbed structure
- legality remains explicit and stable without widening search radius

Qualification Order
1. candidate-specific discovery definition
2. explicit microprogram fixture
3. optional explicit expression fixture only if the shape supports it cleanly
4. parser-backed correctness tests
5. report-only coverage on the default suite
6. replacement qualification on the default suite if coverage is nontrivial
7. comparison against:
  - primitive baseline
  - GENERIC_CONE5_ROOTED
  - WIDE_ROOT_CONE6

Decision Targets
- ADMIT AS PHASE 0 ALTERNATE
- ADMIT AS PHASE 0 PREFERRED
- HOLD FOR LATER
- REJECT

Next Practical Step
- define the rooted matcher for the locality-aware cone shape
- keep the same depth, node-budget, and fail-fast rules
- prefer a region whose best implementation is straight-line microprogram
  execution rather than direct expression execution
