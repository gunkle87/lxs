FUNCTIONAL COMPILATION PHASE 1 CLOSE

Purpose
This document closes the first arithmetic compiled-region branch.

Branch Summary
Phase 1 asked whether bounded arithmetic compiled regions could become a new
winning execution line once the Phase 0 functional compilation machinery was
proven.

What was proven
- explicit multi-output functional regions are real
- bounded arithmetic compiled regions are real
- arithmetic-specialized functional ops improve compiled-region execution
- the row-pair reduce-propagate region is correct and profitable relative to
  the original multiplier once specialized execution is used

What did not happen
- the compiled-region line did not beat the existing arithmetic winners
- it did not overtake:
  - neighborhood
  - neighborhood2
  - packed

Final same-batch EPFL multiplier ordering
- packed:
  - 609,159,011.005506 GEPS
- neighborhood:
  - 472,846,727.814126 GEPS
- neighborhood2:
  - 460,481,478.259717 GEPS
- region1 with HA2 and FA3:
  - 446,827,076.501095 GEPS
- original:
  - 373,589,000.885021 GEPS

Interpretation
- the arithmetic compiled-region branch is valid
- it is not a dead end
- but this exact line is now in diminishing-return territory
- the best delivered arithmetic path is still the packed path

Decision
- close this arithmetic compiled-region branch as:
  - PROVEN ALTERNATE
  - NOT CURRENT WINNER

What is kept
- explicit multi-output functional-region support
- arithmetic-specialized functional ops:
  - MAJ3
  - HA2
  - FA3
- row-pair compiled region as an alternate executable form
- all qualification checkpoints from this branch

What is not recommended
- do not keep tuning the same row-pair region with more small op tweaks
- do not move this branch into recognition
- do not spend more cycles trying to make this exact alternate catch the
  packed winner by incremental edits

Next direction
- optimize the packed arithmetic path directly
- if compiled regions are revisited later, require a materially different
  structural or execution hypothesis
