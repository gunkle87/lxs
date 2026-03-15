FUNCTIONAL COMPILATION CHECKPOINT 01

Candidate
- name: GENERIC_CONE5_ROOTED
- class: structural-combinational
- status: PHASE 0 PROTOTYPE ACTIVE

Purpose
This checkpoint records the first working Phase 0 functional compilation
prototype:
- explicit functional region execution
- parser-backed validation
- bounded report-only cone discovery on real benches

What was implemented
- explicit single-output functional region syntax:
  - FUNC_REGION
- fixed microprogram execution for bounded unary/binary cones
- report-only recognition family:
  - functional
- bounded rooted cone discovery with these limits:
  - root anchored at one output gate
  - max depth: 3
  - max node budget: 8
  - max boundary inputs: 5
  - max internal fanout: 1
  - single output only

Files
- [lxs_types.h](/c:/DEV/LXS/include/lxs_types.h)
- [lxs_compiler.c](/c:/DEV/LXS/src/core/lxs_compiler.c)
- [lxs_engine.c](/c:/DEV/LXS/src/core/lxs_engine.c)
- [lxs_bench.c](/c:/DEV/LXS/src/apps/lxs_bench.c)
- [lxs_test.c](/c:/DEV/LXS/src/apps/lxs_test.c)
- [Tests/Circuits/functional_region_explicit.bench](/c:/DEV/LXS/Tests/Circuits/functional_region_explicit.bench)
- [Tests/Circuits/functional_region_primitive.bench](/c:/DEV/LXS/Tests/Circuits/functional_region_primitive.bench)

Correctness
- full test suite:
  - passed
- explicit parser-backed equivalence:
  - functional_region_explicit vs functional_region_primitive
  - equivalent across all 32 input combinations

Report-only bounded cone discovery
Recognition mask used
- functional only

Recognition mode
- report_only

Observed on c499
- functional_matches: 8
- functional_node_reduction: 16
- functional_gate_equiv: 24
- functional_absorbed_work_share: 0.097561
- telemetry:
  - candidate_roots: 202
  - nodes_visited: 240
  - max_depth: 1
  - abort_shape: 48
  - abort_fanout: 146
  - abort_branch: 0
  - abort_depth: 0
  - abort_node_budget: 0
  - abort_overlap: 0

Observed on c432
- functional_matches: 0
- functional_node_reduction: 0
- functional_gate_equiv: 0
- functional_absorbed_work_share: 0.000000
- telemetry:
  - candidate_roots: 160
  - nodes_visited: 129
  - max_depth: 2
  - abort_shape: 20
  - abort_fanout: 140
  - abort_branch: 0
  - abort_depth: 0
  - abort_node_budget: 0
  - abort_overlap: 0

Interpretation
- Phase 0 machinery is now real:
  - explicit kernel path works
  - accounting works
  - report-only discovery works
  - discovery is bounded and fail-fast on real benches
- The dominant blocker on the current prototype is fanout leakage, not search
  pathology.
- This is the correct outcome for Phase 0:
  - the system can now distinguish structural absence from legality rejection
  - without drifting into graph-search behavior

Decision
- keep the explicit functional region execution path
- keep the functional report-only discovery path
- do not attempt live replacement on real benchmarks yet

Next sensible move
- add one controlled replacement qualification on a synthetic primitive cone
  family
- then decide whether GENERIC_CONE5_ROOTED should be admitted as a Phase 0
  prototype replacement path
