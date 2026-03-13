RECOGNITION PHASE 2 CHECKPOINT 02

Family
Compare/Equality

Purpose
This checkpoint records the first Phase 2 compare/equality retry after widening
expression without widening search radius.

Protocol Revisit
- Root/output anchoring preserved
- Fixed local budget preserved
- Fixed traversal depth preserved
- Strict abort conditions preserved
- Matcher telemetry enabled
- Stress-pack gate required before fresh corpus census

Matcher Shape
- Replace mode remains strict XNOR bank recognition
- Report-only mode now recognizes bounded compare/decode neighborhoods rooted at
  wide AND/OR outputs with 4 to 8 inputs
- No recursive widening
- No fanout-leak broadening

Stress-Pack Gate
Settings
- samples: 1
- iterations: 1
- cycles: 1
- timeout_seconds: 30

Results
- BenchSuites\EPFL\multiplier.bench
  - status: ok
  - elapsed_ms: 18298
  - matches: 0
  - candidate_roots: 0
  - nodes_visited: 0
  - max_depth: 0
- BenchSuites\ISCAS89\s38417.bench
  - status: ok
  - elapsed_ms: 6185
  - matches: 0
  - candidate_roots: 106
  - nodes_visited: 833
  - max_depth: 3
  - abort_shape: 106
- BenchSuites\ISCAS89\s38584.bench
  - status: ok
  - elapsed_ms: 6734
  - matches: 0
  - candidate_roots: 393
  - nodes_visited: 2783
  - max_depth: 3
  - abort_shape: 393
- BenchSuites\ITC99\b22_1.bench
  - status: ok
  - elapsed_ms: 5960
  - matches: 0
  - candidate_roots: 135
  - nodes_visited: 745
  - max_depth: 3
  - abort_shape: 135

Interpretation
- The matcher is now bounded and fail-fast on heavy designs.
- Zero matches on the stress pack reflect structural absence under the bounded
  shape, not runaway search.
- Compare/equality is now corpus-ready for census.

Fresh Compare Census
Settings
- BenchRoot: BenchSuites
- samples: 1
- iterations: 1
- cycles: 1
- timeout_seconds: 30

Results
- match_count: 16
- gate_equiv: 64
- step_share: 0.00177226406734603
- work_share: 0.000100148971595248
- circuits_affected: 1
- timed_out: 4
- errors: 0

Observed Affected Circuit
- c499 local report-only sample:
  - compare_matches: 16
  - compare_gate_equiv: 64
  - compare_absorbed_work_share: 0.260163

Status
HOLD FOR REPLACEMENT QUALIFICATION

Reason
- The family now has real live corpus coverage under the widened bounded
  expression.
- It has cleared the heavy-bench stress gate.
- It has not yet been re-qualified in replacement mode against the suite under
  Phase 2 rules.

Next Allowed Move
- Run compare/equality replacement qualification against the current baseline
- Report:
  - overall delta
  - per-suite delta
  - per-benchmark delta
  - macro_step_share
  - absorbed_work_share
  - family_absorbed_work_share
