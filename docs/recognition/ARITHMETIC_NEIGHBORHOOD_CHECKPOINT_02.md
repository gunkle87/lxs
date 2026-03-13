ARITHMETIC NEIGHBORHOOD CHECKPOINT 02

Purpose
This checkpoint records the first attempt to widen the arithmetic neighborhood
from 4 bits to 8 bits.

Candidate
- RIPPLE_ADD8_NEIGHBORHOOD

Status
- HOLD FOR LATER

What Was Attempted
- explicit RIPPLE_ADD8 kernel
- dedicated explicit and primitive fixtures
- neighborhood8 offline rewrite mode for the vector adder rewriter
- controlled EPFL adder substitution

Observed Outcome
- the 8-bit path was structurally plausible
- but it exposed an execution or harness instability before qualification
  could be completed cleanly
- the earlier shell-level success signal for comparison was not reliable
- standalone qualification paths for the 8-bit form were not stable enough to
  treat the result as valid

Decision
- do not admit RIPPLE_ADD8_NEIGHBORHOOD
- do not keep the 8-bit implementation active in the current tree
- keep RIPPLE_ADD4_NEIGHBORHOOD as the current arithmetic neighborhood ceiling

Interpretation
- this is not evidence that larger arithmetic neighborhoods are a dead end
- it is evidence that the first 8-bit attempt is not stable enough to qualify
- the correct protocol action is to hold, not force the family forward

Current Arithmetic Standing
- explicit RIPPLE_ADD4 kernel: keep
- neighborhood rewrite mode based on RIPPLE_ADD4: keep
- packed RIPPLE_SLICE2 arithmetic path: still the current rewrite winner on
  EPFL adder

Next Allowed Move
- either revisit RIPPLE_ADD8 later with a tighter implementation and explicit
  stability instrumentation
- or move to a different larger-neighborhood concept without carrying the
  unstable add8 code forward
