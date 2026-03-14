FUNCTIONAL COMPILATION CANDIDATE 04

Purpose
This document defines the next Phase 0 candidate after CACHE_CHAIN_CONE7 was
admitted as the preferred bounded functional prototype.

Candidate Name
CACHE_MERGE_CONE8

Class
- structural-combinational
- single-output
- bounded generic cone
- cache-aware
- microprogram-first

Why This Candidate Exists
CACHE_CHAIN_CONE7 proved that one additional strictly local fused step can
improve the suite-level replacement result over CACHE_LOCAL_CONE6 while
preserving the cache-aware, rooted, fail-fast model.

But CACHE_CHAIN_CONE7 still leaves a visible ceiling:
- it preserves chain locality by allowing only one internal path to feed the
  root
- it still hands off one small root-adjacent side computation to primitive
  execution in many real cones
- absorbed work share remains small at suite scale

The next step is to test whether one tightly bounded root-adjacent side merge
can be absorbed without reopening the branching and temp-sprawl failures that
hurt wider candidates.

Design Intent
CACHE_MERGE_CONE8 keeps the same rooted, single-output, fail-fast legality
model as CACHE_CHAIN_CONE7, but allows one additional local side leaf merge at
the root boundary.

The intended region shape is:
- one bounded root gate
- one main internal chain feeding the root
- one bounded side leaf or side temp also feeding the root
- no deeper internal branching
- no reconvergent internal fanout
- no wide-root bias

In plain terms:
- keep the chain
- allow one extra bounded merge at the root
- do not allow a general internal tree

Design Goal
- increase absorbed work over CACHE_CHAIN_CONE7
- preserve cache-aware linear evaluation order for the main path
- absorb one more useful boundary handoff without reopening wide-root costs

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

Additional locality rules
- op_count must be exactly 5
- temp_count must not exceed 4
- root arity remains 2 or 3
- non-root ops must remain unary or binary only
- exactly two root inputs may originate from local work:
  - one main chain temp
  - one side temp or direct side op result
- internal dependency graph must remain merge-local:
  - one dominant chain
  - one bounded side leaf
  - no multi-level sibling branching
  - no internal reconvergence except at the root
- candidate must be rejected if local execution order cannot remain strictly
  forward with fixed temp reuse

Negative Space
Must not match:
- wide-root dominated cones
- arithmetic carry chains
- multi-output regions
- sequential crossings
- internal branching trees with more than one side merge
- reconvergent temp graphs below the root
- any region whose benefit depends on widening search radius

Expected Execution Strategy
- fixed microprogram evaluation only

Expected Benefit Hypothesis
- slightly higher absorbed work than CACHE_CHAIN_CONE7
- better boundary reduction at the root
- still cache-aware enough to avoid the WIDE_ROOT_CONE6 failure mode

What This Candidate Must Prove
- one bounded root-adjacent merge can improve usefulness over CACHE_CHAIN_CONE7
- the locality discipline can survive a limited root merge
- microprogram-first execution benefits from one more fused step without
  reopening large local temp churn

Qualification Order
1. candidate-specific discovery definition
2. explicit microprogram fixture
3. parser-backed correctness tests
4. report-only coverage on the default suite
5. replacement qualification on the default suite if coverage is nontrivial
6. comparison against:
  - primitive baseline
  - CACHE_LOCAL_CONE6
  - CACHE_CHAIN_CONE7
  - GENERIC_CONE5_ROOTED
  - WIDE_ROOT_CONE6

Decision Targets
- ADMIT AS PHASE 0 PREFERRED
- ADMIT AS PHASE 0 ALTERNATE
- HOLD FOR LATER
- REJECT

Next Practical Step
- define the merge-local matcher for the 5-op bounded cone
- keep the same rooted and fail-fast rules
- preserve chain-locality with one bounded root merge, not a general branchy
  cone
