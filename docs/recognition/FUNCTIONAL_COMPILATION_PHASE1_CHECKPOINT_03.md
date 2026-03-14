FUNCTIONAL COMPILATION PHASE 1 CHECKPOINT 03

Candidate
- name: ARITHMETIC_SPECIALIZED_OPS_01
- first specialized op: MAJ3
- target region: ROWPAIR_REDUCE_PROPAGATE4_REGION
- status: KEEP AS ALTERNATE EXECUTION FORM

Purpose
This checkpoint records the first execution-form refinement pass for Phase 1.

What changed
- added MAJ3 to the functional region execution vocabulary
- re-expressed the row-pair reduce-propagate compiled region using:
  - XOR with arity 3
  - MAJ3 for carry and spill majority points
- kept the same bounded region boundary and the same multiplier rewrite mode:
  - region1

Correctness
- full test suite:
  - passed
- multi-output full-adder functional region using MAJ3:
  - passed
- row-pair reduce-propagate functional region:
  - passed across all 4096 input combinations
- EPFL multiplier vs regenerated region1 artifact:
  - equivalent across 512 trials

Local execution comparison
- previous generic row-pair compiled region:
  - 55,259,393.451536 GEPS
- MAJ3-specialized row-pair compiled region:
  - 84,234,516.236092 GEPS

Interpretation
- the specialized arithmetic op materially improves the compiled-region
  execution form
- it does not fully solve the local execution gap
- but it shows that execution vocabulary, not just region shape, is a real
  lever in Phase 1

EPFL multiplier comparison
- original:
  - 390,931,714.937201 GEPS
- region1 specialized:
  - 360,384,670.173003 GEPS
- neighborhood:
  - 372,008,787.025613 GEPS
- neighborhood2:
  - 364,910,490.013627 GEPS
- packed:
  - 463,116,902.492671 GEPS

Interpretation
- the specialized region1 path is better than the earlier generic region1 path
- on this rerun, it is still below the original multiplier
- it still does not beat the best existing arithmetic paths
- neighborhood and neighborhood2 are also below the original on this batch
- packed remains the only clear winner in this comparison set

Decision
- KEEP AS ALTERNATE EXECUTION FORM

Reason
- execution specialization helped enough to justify the concept
- but not enough to promote the compiled region over the current arithmetic
  leaders
- and not enough to make the specialized region a benchmark-level winner on
  this rerun

Next sensible move
- decide whether to add one more arithmetic-specialized functional op
  with the same bounded region shape
- or stop execution-form tuning here and reassess Phase 1 economics
