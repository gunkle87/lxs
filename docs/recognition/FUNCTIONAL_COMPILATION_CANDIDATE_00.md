FUNCTIONAL COMPILATION CANDIDATE 00

Purpose
This document defines the first Phase 0 pilot candidate for functional
compilation.

Candidate Name
GENERIC_CONE5_ROOTED

Class
- structural-combinational
- single-output
- bounded generic cone

Why This Candidate Is First
- it validates the functional compilation machinery without arithmetic-specific
  complexity
- it keeps the first pilot:
  - easy to reason about
  - easy to debug
  - widely applicable
- it lets the project test the region execution model before carry chains,
  reconvergence, or multi-output arithmetic regions are reintroduced

Semantic Contract
- one output
- up to five boundary inputs
- exact deterministic combinational equivalence to the absorbed primitive cone
- no internal state
- no hidden sequential interaction

Legality Boundary
- root anchored at one output gate
- max depth: 3
- max node budget: 8
- max boundary inputs: 5
- max internal fanout: 1
- output count: 1
- strict abort on:
  - fanout leakage
  - branch growth
  - overlap ambiguity
  - budget overflow
  - depth overflow

Negative Space
Must not match:
- any cone with multiple outputs
- any cone crossing sequential boundaries
- any cone requiring open-ended carry or decode interpretation
- any cone whose internal node fanout escapes the region
- any cone whose boundary input count exceeds 5

Expected Execution Strategy
Two allowed forms:
- tiny direct expression evaluation for the smallest cones
- fixed microprogram evaluation for larger cones within the same legality
  limits

Expected Benefit Hypothesis
- not a large benchmark jump by itself
- should prove whether functional compilation can beat stitched primitive
  evaluation on small closed regions at all
- should expose boundary-cost and telemetry behavior early

What This Candidate Must Prove
- bounded discovery is practical
- legality can remain explicit and separate from structure
- runtime execution shape is stable
- accounting remains comparable
- one generic cone class can show measurable value without arithmetic-specific
  special casing

Qualification Order
1. candidate-specific runtime representation
2. dedicated explicit kernel
3. dedicated tests
4. bounded compiler discovery in reporting-only form first
5. replacement on controlled synthetic cases
6. benchmark qualification on selected real circuits

Decision Targets
- ADMIT AS PHASE 0 PROTOTYPE
- HOLD FOR LATER
- REJECT

Next Practical Step
- define the exact region descriptor and execution form for GENERIC_CONE5_ROOTED
- implement it without arithmetic-specific assumptions
- keep it single-output and bounded
