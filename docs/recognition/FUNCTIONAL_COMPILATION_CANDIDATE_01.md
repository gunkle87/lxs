FUNCTIONAL COMPILATION CANDIDATE 01

Purpose
This document defines the next bounded Phase 0 candidate after
GENERIC_CONE5_ROOTED.

Candidate Name
WIDE_ROOT_CONE6

Class
- structural-combinational
- single-output
- bounded generic cone
- expression-biased

Why This Candidate Exists
GENERIC_CONE5_ROOTED proved that bounded functional compilation is real, but
its direct-expression execution form was not profitable enough to become the
default Phase 0 execution path.

The most likely reason is shape, not concept:
- the current prototype is still mostly a stitched binary cone
- expression execution pays extra recursion and memo cost
- there is not enough structural collapse at the root to amortize that cost

The next candidate should therefore be richer in expression structure without
widening search radius.

Design Intent
WIDE_ROOT_CONE6 keeps the same rooted, single-output, fail-fast legality model,
but changes the structural shape:

- one wide root gate
  - AND, OR, XOR, NAND, NOR, or XNOR
  - root input count: 3 or 4
- each root input may be:
  - a boundary input
  - or the output of one bounded local preprocess gate
- preprocess gates remain unary or binary only
- internal fanout remains fully contained

This gives the functional compiler a slightly richer cone to absorb while still
remaining small and explainable.

Why It May Fit Direct Evaluation Better
- the root performs more real work per compiled region
- the shape is shallower and more tree-like at the top
- fewer internal temporary handoffs are needed than in a purely chained binary
  cone
- the wide root can be evaluated as a compact fold once leaf values are ready

This candidate is meant to test whether expression execution benefits more from
wide-root structure than from the earlier binary-only rooted cone.

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
- root input count must be 3 or 4
- max depth: 3
- max node budget: 8
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

Negative Space
Must not match:
- purely arithmetic carry chains
- multi-output regions
- any cone crossing sequential boundaries
- any cone whose internal node fanout escapes the region
- any root wider than 4 inputs
- any region whose boundary input count exceeds 6
- any region requiring open-ended graph growth to complete the shape

Expected Execution Strategy
Two allowed forms:
- direct expression evaluation
  - expected preferred form for this candidate
- fixed microprogram evaluation
  - fallback if expression execution does not qualify

Expected Benefit Hypothesis
- more favorable to direct evaluation than GENERIC_CONE5_ROOTED
- still bounded enough to remain cheap to discover
- may improve useful absorbed work share on mixed logic/control circuits more
  than the current prototype

What This Candidate Must Prove
- expression-favoring structure can outperform the current binary-only rooted
  prototype
- a wide-root bounded cone can still be discovered with fail-fast behavior
- legality remains explicit and stable
- the richer expression shape does not turn the matcher into a wider search

Qualification Order
1. candidate-specific discovery definition
2. explicit expression and microprogram fixtures
3. parser-backed correctness tests
4. report-only coverage on the default suite
5. replacement qualification on the default suite if coverage is nontrivial
6. comparison against:
  - primitive baseline
  - explicit expression
  - explicit microprogram
  - current GENERIC_CONE5_ROOTED baseline

Decision Targets
- ADMIT AS PHASE 0 ALTERNATE
- ADMIT AS PHASE 0 PREFERRED
- HOLD FOR LATER
- REJECT

Next Practical Step
- define the rooted matcher for the wide-root shape
- keep the same depth, node-budget, and fail-fast rules
- build explicit fixtures first before touching suite replacement
