FUNCTIONAL COMPILATION CHECKPOINT 04

Candidate
- name: GENERIC_CONE5_ROOTED
- class: structural-combinational
- status: KEEP KERNEL, DEFER EXPRESSION DEFAULT

Purpose
This checkpoint records the first explicit execution-form comparison inside
Functional Compilation Phase 0.

Question
Can a tiny direct-expression execution form outperform the existing fixed
microprogram execution form on the same bounded cone class without widening
search radius or legality?

Guard rails kept unchanged
- rooted
- single-output
- fixed depth
- fixed node budget
- strict fanout abort
- no wider discovery radius

Execution forms compared
- primitive baseline:
  - [functional_region_primitive.bench](/c:/DEV/LXS/Tests/Circuits/functional_region_primitive.bench)
- explicit expression form:
  - [functional_region_explicit.bench](/c:/DEV/LXS/Tests/Circuits/functional_region_explicit.bench)
- explicit microprogram form:
  - [functional_region_micro.bench](/c:/DEV/LXS/Tests/Circuits/functional_region_micro.bench)

Correctness
- full test suite:
  - passed
- parser-backed equivalence:
  - explicit expression vs primitive:
    - passed
  - explicit microprogram vs primitive:
    - passed

Execution-form qualification
Settings
- samples: 5
- sample_select: median
- iterations: 1
- cycles: 100000

Results
- primitive baseline:
  - 30,983,733.367427 GEPS
- explicit expression:
  - 21,524,007.773076 GEPS
- explicit microprogram:
  - 27,237,010.172213 GEPS

Interpretation
- The expression execution form is real and correct.
- On the bounded Phase 0 cone, it is slower than:
  - the current microprogram execution form
  - the primitive baseline
- That means the direct-expression path does not qualify as the new default
  execution form for Phase 0 at this time.

Decision
- KEEP KERNEL, DEFER EXPRESSION DEFAULT

What remains true
- The second execution form is now available for controlled experimentation.
- The current default Functional Phase 0 path should remain microprogram-based.
- The default-suite replacement decision from Checkpoint 03 remains the active
  suite-level decision.

Next sensible move
- keep expression execution as an explicit experimental path only
- continue widening functional expression only if a future bounded cone class
  suggests a better fit for direct evaluation than the current prototype
