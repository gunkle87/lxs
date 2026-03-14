FUNCTIONAL COMPILATION CHECKPOINT 05

Candidate
- name: WIDE_ROOT_CONE6
- class: structural-combinational
- status: KEEP KERNEL, DEFER DEFAULT REPLACEMENT

Purpose
This checkpoint records the first qualification pass for the richer bounded
Phase 0 candidate after GENERIC_CONE5_ROOTED.

Question
Can a wider rooted cone with a 3-input or 4-input root gate improve
expression-oriented functional compilation enough to justify widening the
default Phase 0 replacement path?

Guard rails retained
- rooted
- single-output
- fixed depth
- fixed node budget
- strict fanout abort
- no wider search radius

What changed
- Functional region ops now support arity up to 4.
- The richer candidate allows:
  - one 3-input or 4-input root gate
  - bounded unary/binary preprocess gates below it
- Explicit fixtures were added for:
  - primitive baseline
  - expression execution
  - microprogram execution

Files
- [lxs_types.h](/c:/DEV/LXS/lxs_types.h)
- [lxs_compiler.c](/c:/DEV/LXS/lxs_compiler.c)
- [lxs_engine.c](/c:/DEV/LXS/lxs_engine.c)
- [lxs_test.c](/c:/DEV/LXS/lxs_test.c)
- [functional_region_wide_primitive.bench](/c:/DEV/LXS/Tests/Circuits/functional_region_wide_primitive.bench)
- [functional_region_wide_explicit.bench](/c:/DEV/LXS/Tests/Circuits/functional_region_wide_explicit.bench)
- [functional_region_wide_micro.bench](/c:/DEV/LXS/Tests/Circuits/functional_region_wide_micro.bench)

Correctness
- full test suite:
  - passed
- parser-backed equivalence:
  - explicit expression vs primitive:
    - passed across all 64 input combinations
  - explicit microprogram vs primitive:
    - passed across all 64 input combinations
  - recognition replacement vs primitive:
    - passed across all 64 input combinations

Execution-form qualification
Settings
- samples: 5
- sample_select: median
- iterations: 1
- cycles: 100000

Results
- primitive baseline:
  - 68,913,927.029062 GEPS
- explicit expression:
  - 24,382,959.353712 GEPS
- explicit microprogram:
  - 28,679,386.210137 GEPS

Interpretation
- The richer cone shape is valid.
- Direct expression execution is still not profitable.
- Microprogram execution remains better than expression on this candidate, but
  still slower than the primitive baseline.

Report-only default-suite coverage
- match_count: 31
- gate_equiv: 123
- step_share: 0.0109501942776404
- work_share: 0.000800489404904462
- circuits_affected: 9
- timed_out: 0
- errors: 0

Interpretation
- WIDE_ROOT_CONE6 does broaden live coverage beyond the earlier bounded
  functional prototype.
- The richer expression shape is visible in real circuits.

Default-suite replacement qualification
- overall_off: 427,760,163.821856 GEPS
- overall_on: 415,928,868.095153 GEPS
- overall_delta: -11,831,295.726704 GEPS

Per-suite delta
- ISCAS85:
  - -20,981,470.226584
- ISCAS89:
  - -131,217,023.462704
- ITC99:
  - +10,845,179.687400
- EPFL:
  - -24,898,164.643142

Interpretation
- Coverage is nontrivial.
- Default replacement is still not profitable enough to admit.
- The candidate is useful for continuing Phase 0 exploration, but not as the
  default functional replacement path.

Decision
- KEEP KERNEL, DEFER DEFAULT REPLACEMENT

Next sensible move
- do not widen search radius
- keep the wider rooted cone as a bounded alternate
- if Phase 0 continues, focus next on a shape that can improve runtime locality
  or reduce boundary handoff cost more directly than WIDE_ROOT_CONE6
