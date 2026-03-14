FUNCTIONAL COMPILATION PHASE 1 CHECKPOINT 01

Purpose
This checkpoint records the first Phase 1 architectural prerequisite:
explicit multi-output functional regions.

Why This Checkpoint Exists
Phase 0 only needed single-output functional regions.
Arithmetic compiled regions do not fit that shape cleanly.

The first Phase 1 step therefore is not immediate benchmark substitution.
The first Phase 1 step is to extend the explicit functional-region machinery so
it can represent closed arithmetic boundaries with more than one output.

What Landed
- explicit multi-output functional region support in the parser
- explicit multi-output functional region support in the compiled plan
- explicit multi-output functional region execution in the runtime

New explicit syntax capability
- destinations may now be assigned explicitly inside FUNC_MICRO or FUNC_REGION
  bodies using:
  - tN = OP(...)
  - oN = OP(...)
- single-output legacy syntax remains supported
- multi-output expression execution is not allowed
- multi-output functional regions default to microprogram execution

Validation Artifact
- primitive reference:
  - Tests/Circuits/functional_region_full_adder_primitive.bench
- explicit multi-output compiled region:
  - Tests/Circuits/functional_region_full_adder_micro.bench

Correctness
- full test suite:
  - passed
- explicit multi-output arithmetic fixture:
  - passed across all 8 input combinations

Interpretation
- the functional compilation layer now supports explicit arithmetic-style
  multi-output boundaries
- this does not yet qualify any arithmetic compiled region on benchmarks
- it removes the main representation blocker that kept Phase 1 from starting

Decision
- PHASE 1 FOUNDATION READY

Next Sensible Move
- implement the first true Phase 1 arithmetic compiled-region candidate:
  - ROWPAIR_REDUCE_PROPAGATE4_REGION
- keep it explicit first
- then use the existing EPFL multiplier rewrite harness for controlled
  qualification
