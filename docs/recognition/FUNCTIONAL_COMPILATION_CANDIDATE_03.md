FUNCTIONAL COMPILATION CANDIDATE 03

Purpose
This document defines the next Phase 0 candidate after CACHE_LOCAL_CONE6 was
admitted as the default prototype.

Candidate Name
CACHE_CHAIN_CONE7

Class
- structural-combinational
- single-output
- bounded generic cone
- cache-aware
- microprogram-first

Why This Candidate Exists
CACHE_LOCAL_CONE6 proved that a tighter local working set can produce a
positive suite-level replacement result even when richer expression-heavy cones
do not.

But CACHE_LOCAL_CONE6 still leaves one likely source of missed value:
- it caps local work too early
- it often stops one useful operation short of absorbing the next obvious local
  preprocess step
- its positive suite result still comes with small absorbed work share

The next step is not to widen the root again.
The next step is to test whether one more strictly local operation can be
absorbed without breaking the cache-aware shape.

Design Intent
CACHE_CHAIN_CONE7 keeps the same rooted, single-output, fail-fast legality
model as CACHE_LOCAL_CONE6, but allows one extra bounded internal operation
only when the region still behaves like a linear local chain.

The intended region shape is:
- one bounded root gate
- one primary internal temp feeding the root path
- one secondary local preprocess step beyond CACHE_LOCAL_CONE6
- optional side leaf at the root boundary
- no wide-root bias
- no branching internal tree

In plain terms:
- one more fused local step
- not a richer search
- not a wider root
- not a temp explosion

Design Goal
- increase absorbed work modestly over CACHE_LOCAL_CONE6
- preserve linear evaluation order
- preserve short temp live ranges
- preserve bounded and fail-fast behavior

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
- max node budget: 7
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

Additional locality rules
- op_count must be exactly 4
- temp_count must not exceed 3
- root arity must remain 2 or 3
- non-root ops must remain unary or binary only
- every internal temp must feed exactly one later internal op, except:
  - one final temp may feed the root directly
- internal dependency graph must be chain-like
  - no sibling internal branches
  - no reconvergent internal fanout
- candidate must be rejected if local execution order cannot remain strictly
  forward with fixed temp reuse

Negative Space
Must not match:
- wide-root dominated cones
- arithmetic carry chains
- multi-output regions
- sequential crossings
- internal branching trees
- reconvergent internal temp graphs
- any region whose benefit depends on widening search radius

Expected Execution Strategy
- fixed microprogram evaluation only

Why Expression Execution Is Not Part Of This Candidate
Phase 0 already showed:
- expression execution loses on the existing bounded candidates
- microprogram evaluation is the better baseline for locality-focused work

Expected Benefit Hypothesis
- better absorbed work than CACHE_LOCAL_CONE6
- similar or better locality profile
- lower risk than WIDE_ROOT_CONE6
- more likely to improve replacement profitability through one additional fused
  local step rather than richer expression shape

What This Candidate Must Prove
- one more local fused step can improve usefulness without reopening temp churn
- chain-like locality scales better than wide-root richness
- microprogram-first execution can benefit from slightly larger but still
  linear local cones

Qualification Order
1. candidate-specific discovery definition
2. explicit microprogram fixture
3. parser-backed correctness tests
4. report-only coverage on the default suite
5. replacement qualification on the default suite if coverage is nontrivial
6. comparison against:
  - primitive baseline
  - GENERIC_CONE5_ROOTED
  - CACHE_LOCAL_CONE6
  - WIDE_ROOT_CONE6

Decision Targets
- ADMIT AS PHASE 0 PREFERRED
- ADMIT AS PHASE 0 ALTERNATE
- HOLD FOR LATER
- REJECT

Next Practical Step
- define the chain-local matcher for the 4-op bounded cone
- keep the same rooted and fail-fast rules
- preserve strict locality over raw expressiveness
