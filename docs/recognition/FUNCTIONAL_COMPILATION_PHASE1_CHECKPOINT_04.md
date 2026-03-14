FUNCTIONAL COMPILATION PHASE 1 CHECKPOINT 04

Candidate
- name: ARITHMETIC_SPECIALIZED_OPS_02
- specialized ops:
  - HA2
  - FA3
- target region: ROWPAIR_REDUCE_PROPAGATE4_REGION
- status: KEEP AS ALTERNATE EXECUTION FORM

Purpose
This checkpoint records one final tightly scoped arithmetic-specialized
execution pass for Phase 1A.

What changed
- extended the functional region layer with two-output arithmetic ops:
  - HA2
  - FA3
- re-expressed the bounded row-pair region using those ops
- kept the same bounded region boundary and the same rewrite mode:
  - region1

Correctness
- full test suite:
  - passed
- full-adder functional region using FA3:
  - passed
- row-pair reduce-propagate functional region using FA3 and HA2:
  - passed across all 4096 input combinations
- EPFL multiplier vs regenerated region1 artifact:
  - equivalent across 512 trials

Local execution comparison
- previous generic row-pair compiled region:
  - 55,259,393.451536 GEPS
- MAJ3-specialized row-pair compiled region:
  - 84,234,516.236092 GEPS
- HA2/FA3-specialized row-pair compiled region:
  - 94,925,399.646845 GEPS

Interpretation
- dual-output arithmetic-specialized ops improve the local compiled-region
  execution form again
- the direction is real
- the gain is incremental, not transformational

Same-batch EPFL multiplier comparison
- original:
  - 373,589,000.885021 GEPS
- region1 with HA2 and FA3:
  - 446,827,076.501095 GEPS
- neighborhood:
  - 472,846,727.814126 GEPS
- neighborhood2:
  - 460,481,478.259717 GEPS
- packed:
  - 609,159,011.005506 GEPS

Interpretation
- the dual-output specialized region1 path is now clearly profitable against
  the original multiplier on the same batch
- it still does not beat:
  - neighborhood
  - neighborhood2
  - packed
- packed remains the arithmetic winner by a wide margin

Decision
- KEEP AS ALTERNATE EXECUTION FORM

Reason
- the final tightly scoped pass improved the compiled-region line enough to
  validate the specialized-op idea
- but not enough to displace the existing arithmetic leaders

What this means for Phase 1
- Phase 1 is not dead
- but continued tuning of this exact region or execution line is now likely to
  produce diminishing returns
- any further continuation should require a materially different structural or
  execution hypothesis, not another small specialization step
