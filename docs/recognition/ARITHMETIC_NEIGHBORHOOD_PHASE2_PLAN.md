ARITHMETIC NEIGHBORHOOD PHASE 2 PLAN

Purpose
This document defines the next arithmetic neighborhood direction after
RIPPLE_ADD4_NEIGHBORHOOD qualification and the hold decision on
RIPPLE_ADD8_NEIGHBORHOOD.

Reason For Pivot
- RIPPLE_ADD4 proved that larger bounded arithmetic neighborhoods can be
  profitable.
- RIPPLE_ADD8 did not fail on conceptual value; it failed on stability during
  qualification.
- Simply widening the same serial ripple shape is not the right next move for
  the current architecture.

Current Stable Arithmetic Standing
- explicit RIPPLE_ADD4 kernel: keep
- vector-adder neighborhood rewrite mode: keep
- packed RIPPLE_SLICE2 arithmetic rewrite: still the current winner on EPFL
  adder

Phase 2 Objective
Move to a larger arithmetic neighborhood concept that:
- absorbs more local structure than RIPPLE_ADD4
- does not extend a long serial carry chain blindly
- stays rooted, bounded, and fail-fast
- remains suitable for explicit-kernel-first qualification

Next Candidate Direction
CARRY_SAVE_ROW4_NEIGHBORHOOD

Concept
- bounded local reduction neighborhood
- operates on a small arithmetic row or column group
- compresses multiple add terms into:
  - local sum outputs
  - local carry outputs
- postpones long carry propagation to the surrounding structure

Why This Direction
- arithmetic wins so far are strongest when they reduce local boundary count
  without forcing a larger fragile serial path
- multiplier and adder-derived workloads already justify arithmetic as the best
  neighborhood pilot family
- carry-save style local reduction is a more natural larger-neighborhood step
  than continuing to double ripple width

What This Candidate Must Not Become
- no broad multiplier recognition project
- no unbounded adder-tree search
- no global reassociation of arithmetic structure
- no widening of recognition radius beyond the fixed neighborhood boundary

Kernel-First Rule
The next arithmetic phase remains:
- explicit kernel first
- dedicated tests
- controlled offline substitution
- isolated benchmark qualification
- no recognition work until the kernel path proves value

Pilot Targets
- EPFL multiplier
- selected arithmetic subregions only

Secondary Reference
- c6288 only if needed as a historical arithmetic check, not as the primary
  proof point

Success Criteria
- stable explicit execution path
- controlled offline substitution with proven equivalence
- positive isolated benchmark delta on a trusted arithmetic target
- clear structural advantage over the current stitched local macro path

Stopping Conditions
- instability similar to RIPPLE_ADD8
- neighborhood requires widening the search radius
- structural boundary cannot be defined cleanly
- profitability does not exceed the current stitched arithmetic path

Decision
Phase 2 arithmetic should pivot from wider serial ripple neighborhoods to a
bounded local reduction neighborhood, starting with a carry-save row concept.
