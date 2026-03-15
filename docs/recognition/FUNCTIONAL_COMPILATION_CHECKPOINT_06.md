FUNCTIONAL COMPILATION CHECKPOINT 06

Candidate
- name: CACHE_LOCAL_CONE6
- class: structural-combinational
- status: HOLD FOR REPLACEMENT QUALIFICATION

Purpose
This checkpoint records the first matcher and report-only qualification pass for
the cache-aware Phase 0 candidate after WIDE_ROOT_CONE6.

Question
Can a stricter locality-aware bounded cone preserve useful coverage while
positioning the functional family for better replacement profitability than the
wider-root candidate?

Guard rails retained
- rooted
- single-output
- fixed depth
- fixed node budget
- strict fanout abort
- no wider search radius

Candidate-specific locality rules
- root input count limited to 2 or 3
- total op_count fixed at 3
- temp_count must not exceed 2
- non-root ops remain unary or binary only
- each internal temp may be consumed only once

Files
- [lxs_types.h](/c:/DEV/LXS/include/lxs_types.h)
- [lxs_compiler.c](/c:/DEV/LXS/src/core/lxs_compiler.c)
- [lxs_engine.c](/c:/DEV/LXS/src/core/lxs_engine.c)
- [lxs_test.c](/c:/DEV/LXS/src/apps/lxs_test.c)
- [functional_region_cache_primitive.bench](/c:/DEV/LXS/Tests/Circuits/functional_region_cache_primitive.bench)
- [functional_region_cache_micro.bench](/c:/DEV/LXS/Tests/Circuits/functional_region_cache_micro.bench)

Correctness
- full test suite:
  - passed
- parser-backed explicit equivalence:
  - functional_region_cache_micro vs functional_region_cache_primitive:
    - passed across all 32 input combinations
- report-only recognition validation:
  - cache-local primitive fixture:
    - match_count = 1
    - comb_gate_count unchanged
- negative validation:
  - wide-root primitive fixture now reports zero matches under the cache-local
    matcher

Local execution qualification
Settings
- samples: 5
- sample_select: median
- iterations: 1
- cycles: 100000

Results
- primitive baseline:
  - 44,333,614.340397 GEPS
- explicit microprogram:
  - 33,894,093.181293 GEPS

Interpretation
- The cache-local shape is valid and parser-backed.
- On the local synthetic target, the explicit microprogram form is still slower
  than the primitive baseline.
- Candidate 02 therefore has not yet demonstrated local execution superiority.

Report-only default-suite coverage
- match_count: 29
- gate_equiv: 117
- step_share: 0.0102437301306959
- work_share: 0.000761441141250586
- circuits_affected: 8
- timed_out: 0
- errors: 0

Interpretation
- Coverage remains nontrivial.
- Coverage is slightly narrower than WIDE_ROOT_CONE6, which is expected under
  the stricter locality rules.
- The candidate is bounded, fail-fast, and corpus-ready.

Decision
- HOLD FOR REPLACEMENT QUALIFICATION

Next sensible move
- run default-suite replacement qualification for CACHE_LOCAL_CONE6
- compare directly against:
  - GENERIC_CONE5_ROOTED
  - WIDE_ROOT_CONE6
- if replacement still loses materially, Phase 0 should stop widening generic
  cone families and return to architecture planning
