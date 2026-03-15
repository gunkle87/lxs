FUNCTIONAL COMPILATION CHECKPOINT 02

Candidate
- name: GENERIC_CONE5_ROOTED
- class: structural-combinational
- status: ADMIT AS PHASE 0 PROTOTYPE REPLACEMENT

Purpose
This checkpoint records the first controlled replacement qualification for the
Phase 0 bounded functional compilation prototype.

Controlled qualification target
- synthetic primitive cone family:
  - [functional_region_primitive.bench](/c:/DEV/LXS/Tests/Circuits/functional_region_primitive.bench)
- explicit region reference:
  - [functional_region_explicit.bench](/c:/DEV/LXS/Tests/Circuits/functional_region_explicit.bench)

What was fixed
- recognized functional regions were being counted correctly but then lost during
  plan assembly
- primitive comb gate accounting was also being computed before recognized
  functional replacement marks were finalized
- [lxs_compiler.c](/c:/DEV/LXS/src/core/lxs_compiler.c) now:
  - preserves recognized functional region count through allocation
  - recomputes comb gate count after replacement marks are final

Files
- [lxs_compiler.c](/c:/DEV/LXS/src/core/lxs_compiler.c)
- [lxs_types.h](/c:/DEV/LXS/include/lxs_types.h)
- [lxs_bench.c](/c:/DEV/LXS/src/apps/lxs_bench.c)
- [lxs_test.c](/c:/DEV/LXS/src/apps/lxs_test.c)
- [Tests/Circuits/functional_region_explicit.bench](/c:/DEV/LXS/Tests/Circuits/functional_region_explicit.bench)
- [Tests/Circuits/functional_region_primitive.bench](/c:/DEV/LXS/Tests/Circuits/functional_region_primitive.bench)

Correctness
- full test suite:
  - passed
- replacement-mode synthetic qualification:
  - functional recognition count: 1
  - comb gate count after replacement: 0
- explicit parser-backed equivalence:
  - functional_region_explicit vs functional_region_primitive
  - equivalent across all 32 input combinations

Controlled replacement qualification
Settings
- samples: 5
- sample_select: median
- iterations: 1
- cycles: 100000

Primitive-equivalent baseline
- [functional_region_explicit.bench](/c:/DEV/LXS/Tests/Circuits/functional_region_explicit.bench)
- GEPS: 26,274,475.409730

Replacement path
- [functional_region_primitive.bench](/c:/DEV/LXS/Tests/Circuits/functional_region_primitive.bench)
- recognition mask: functional only
- recognition mode: replace
- GEPS: 34,046,955.254715

Observed replacement structure
- functional_matches: 1
- functional_node_reduction: 3
- functional_gate_equiv: 4
- comb_gates after replacement: 0
- chunks after replacement: 0

Interpretation
- The Phase 0 prototype replacement path is now real, not just report-only.
- The bounded rooted cone can be:
  - discovered
  - compiled into one functional region object
  - executed instead of primitive gates
  - benchmarked under primitive-equivalent accounting
- On the synthetic controlled target, replacement is beneficial.
- This is enough to admit GENERIC_CONE5_ROOTED as a Phase 0 prototype
  replacement path.

Decision
- ADMIT AS PHASE 0 PROTOTYPE REPLACEMENT

Scope limits
- single output only
- rooted at one output gate
- max depth: 3
- max node budget: 8
- max boundary inputs: 5
- max internal fanout: 1
- bounded unary/binary gate vocabulary only

Next sensible move
- use this prototype admission as the baseline for widening functional
  compilation carefully
- do not widen search radius
- only widen expression within the same bounded rooted legality model
