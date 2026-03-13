RECOGNITION PHASE 2 COMPARE CLOSE

Purpose
This note closes the current compare and equality retry under the Phase 2
recognition rules and records why the family is deferred again.

Status
- kernel valid
- recognizer bounded
- replacement not profitable
- deferred

What Was Proven
- The compare family is no longer blocked by matcher cost.
- The widened Phase 2 compare matcher is rooted, bounded, and fail-fast.
- Stress-pack telemetry showed shape rejection rather than search blow-up.
- The compare family has real structural availability under widened expression.

What Was Tried
- strict XNOR_BANK4 replacement
- bounded compare-literal AND4 replacement
- bounded OR4-over-AND4 compare neighborhood replacement

All three were kept inside:
- fixed root or output boundary
- fixed local budget
- fixed traversal depth
- strict abort on branch and fanout leakage

What Happened
The compare family improved structurally at each retry:
- more matched work
- more absorbed primitive-equivalent work
- larger local neighborhood replacement

But the actual affected benchmark still lost.

Latest isolated c499 result
- off: 633,922,571.275574 GEPS
- on: 496,428,108.411709 GEPS
- delta: -137,494,462.863865 GEPS

Latest compare neighborhood profile on c499
- compare_matches: 6
- compare_node_reduction: 44
- compare_gate_equiv: 50
- compare_absorbed_work_share: 0.203252

Interpretation
- The current compare kernels are real.
- The current compare recognizer is bounded and usable.
- The current compare replacement boundary is still not profitable enough in
  the present execution model.
- The family has reached a practical local ceiling for the current architecture.

Decision
KEEP KERNEL, DEFER RECOGNITION

Reason
- This is no longer a matcher-boundedness problem.
- It is now a profitability problem.
- Further small compare variants are unlikely to change the outcome materially.

Next Allowed Move
- Do not continue adding small compare variants.
- Shift to larger compiled neighborhoods as the next architecture phase.
- Preserve compare kernels and bounded matcher work as future assets, not as the
  immediate optimization path.
