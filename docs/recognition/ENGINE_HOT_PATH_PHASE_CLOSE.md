ENGINE HOT-PATH PHASE CLOSE

Purpose
This document closes the engine hot-path optimization phase.

What this phase tested
- engine-side arithmetic multi-macro hot-loop tuning on top of the kept packed
  arithmetic baseline
- no generator or recognition changes were admitted in this phase

What was learned
- the engine hot path does still contain real local headroom
- the strongest local signal was reduced repeated input-rail fetches inside
  `RIPPLE_SLICE2`
- but local artifact wins did not translate into a trustworthy suite-level
  default win

Why the phase closed without a kept code change
- Candidate A improved the packed adder and packed multiplier artifacts
- thresholded variants reduced the damage on smaller arithmetic cases
- after serial suite validation, the best candidate only tied the suite
  baseline within noise
- that is not enough to justify carrying a new default engine path

Final decision
- CLOSE the engine hot-path phase without admitting a new default runtime change
- KEEP the codebase on the packed baseline protected before this phase

Value produced by the phase
- it ruled out a tempting but non-credible hot-loop change
- it established a better admission rule for future runtime work:
  - artifact wins are not enough
  - stable serial suite-level validation is required

Current standing
- packed arithmetic remains the strongest arithmetic lane
- the kept packed-path winners from the prior phase remain the active baseline
- this phase did not weaken that baseline

Next sensible move
- do not keep grinding micro-tuning in the same arithmetic loop shape
- the next phase should start from a different bottleneck class with stronger
  suite-level leverage than arithmetic input caching alone
