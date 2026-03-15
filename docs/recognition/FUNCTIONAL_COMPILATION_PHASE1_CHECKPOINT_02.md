FUNCTIONAL COMPILATION PHASE 1 CHECKPOINT 02

Candidate
- name: ROWPAIR_REDUCE_PROPAGATE4_REGION
- class: arithmetic
- type: bounded compiled region
- status: KEEP AS ALTERNATE REGION

Purpose
This checkpoint records the first explicit arithmetic compiled-region
qualification pass for Functional Compilation Phase 1.

What was implemented
- explicit multi-output FUNC_MICRO region for the bounded row-pair
  reduce-plus-propagate window
- parser/runtime support for larger explicit functional regions:
  - 12 boundary inputs
  - 6 outputs
  - expanded op and temp budgets
- multiplier rewrite mode:
  - region1

Files
- [lxs_types.h](/c:/DEV/LXS/include/lxs_types.h)
- [lxs_compiler.c](/c:/DEV/LXS/src/core/lxs_compiler.c)
- [lxs_engine.c](/c:/DEV/LXS/src/core/lxs_engine.c)
- [lxs_test.c](/c:/DEV/LXS/src/apps/lxs_test.c)
- [Tests/Circuits/functional_region_rowpair_reduce_propagate4.bench](/c:/DEV/LXS/Tests/Circuits/functional_region_rowpair_reduce_propagate4.bench)
- [tools/gen_vector_multiplier_macro.ps1](/c:/DEV/LXS/tools/gen_vector_multiplier_macro.ps1)
- [tools/LxsRewriteLib.ps1](/c:/DEV/LXS/tools/LxsRewriteLib.ps1)
- [tools/lxs_rewrite.ps1](/c:/DEV/LXS/tools/lxs_rewrite.ps1)

Correctness
- full test suite:
  - passed
- explicit functional region vs primitive reduce-propagate fixture:
  - passed across all 4096 input combinations
- EPFL multiplier vs generated region1 artifact:
  - equivalent across 512 trials

Generated artifact
- [multiplier_macro_region1.bench](/c:/DEV/LXS/Benchmarks/Generated/multiplier_macro_region1.bench)

Local execution qualification
- primitive reduce_propagate4:
  - 102,833,506.222663 GEPS
- explicit REDUCE_PROPAGATE4 macro:
  - 174,346,195.358157 GEPS
- explicit compiled region:
  - 55,259,393.451536 GEPS

Interpretation
- the compiled-region expression of this arithmetic window is not a local A/B
  winner
- it is substantially slower than the dedicated arithmetic kernel
- it is also slower than the primitive benchmark fixture in isolation

Controlled EPFL multiplier results
- original:
  - 341,258,937.160490 GEPS
- carry-save row neighborhood:
  - 367,151,840.235337 GEPS
- neighborhood2 / dedicated REDUCE_PROPAGATE4:
  - 379,353,951.313771 GEPS
- current packed slice2 path:
  - 480,292,198.474141 GEPS
- phase 1 region1 compiled region:
  - 355,596,833.842380 GEPS

Interpretation
- the first arithmetic compiled region is a real profitable rewrite versus the
  original multiplier
- it does not beat the existing neighborhood rewrite forms
- it does not beat the current packed slice2 arithmetic path
- so the compiled-region layer is valid for arithmetic, but this first region
  shape is not the new arithmetic winner

Decision
- keep ROWPAIR_REDUCE_PROPAGATE4_REGION as an alternate compiled region
- do not promote region1 over:
  - neighborhood
  - neighborhood2
  - packed

Reason
- correctness is clean
- the bounded arithmetic compiled-region model is now proven on a real
  benchmark path
- profitability exists relative to the original multiplier
- but the current region still carries more execution overhead than the best
  existing arithmetic rewrite forms

Next sensible move
- do not move to recognition
- compare where the current region is losing:
  - boundary cost
  - temp schedule cost
  - lack of arithmetic-specialized execution
- then decide whether Phase 1 should:
  - specialize arithmetic functional ops
  - or choose a different arithmetic compiled-region shape
