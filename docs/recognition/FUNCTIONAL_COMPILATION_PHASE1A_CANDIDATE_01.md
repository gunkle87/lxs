FUNCTIONAL COMPILATION PHASE 1A CANDIDATE 01

Purpose
This document defines the first execution-focused follow-up candidate after the
first arithmetic compiled-region pilot proved structurally valid but slower
than the best existing arithmetic rewrite paths.

Candidate Name
ARITHMETIC_SPECIALIZED_OPS_01

Class
- arithmetic
- functional compilation
- execution-form refinement

Why This Candidate Exists
The first Phase 1 region proved an important point:
- the bounded arithmetic compiled-region model is real
- it is profitable relative to the original multiplier
- but generic functional microprogram execution is still too expensive to beat
  the dedicated arithmetic neighborhood and packed paths

That suggests the next bottleneck is not region legality.
It suggests the bottleneck is execution vocabulary.

Design Intent
Keep the same bounded arithmetic region shape, but reduce the internal
microprogram length by introducing arithmetic-specialized functional ops that
still preserve:
- fixed execution shape
- bounded legality
- explicit accounting
- no recognition work

Initial Specialized Ops
- MAJ3
  - three-input majority
  - intended for full-adder carry and carry-propagate majority points

Why MAJ3 Goes First
- it collapses the most repetitive arithmetic substructure in the current
  row-pair compiled region
- it shortens carry construction without widening the region
- it is still small enough to explain and audit cleanly

What This Candidate Must Prove
- arithmetic-specialized functional ops can improve the compiled-region
  runtime materially without changing the region boundary
- Phase 1 can advance by execution specialization before it advances by
  region-shape expansion

Qualification Path
1. add the specialized functional op to the explicit region layer
2. re-express the existing row-pair compiled region using that op
3. rerun the same EPFL multiplier qualification against:
  - region1
  - neighborhood
  - neighborhood2
  - packed
4. decide whether arithmetic-specialized ops justify further expansion

Decision Targets
- KEEP AS ALTERNATE EXECUTION FORM
- ADMIT AS PREFERRED EXECUTION FORM
- HOLD FOR LATER
- REJECT
