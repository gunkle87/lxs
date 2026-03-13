NEIGHBORHOOD PHASE PLAN

Purpose
This document defines the next architecture phase after the current recognition
phase reaches its practical limit on small and medium local macro replacements.

Phase Conclusion That Motivates This Shift
- parity recognition is useful but still narrow
- several deferred families became bounded and measurable
- compare and equality now has real structural coverage
- but larger local compare neighborhoods still lose on the actual benchmark

Interpretation
The next useful step is not another family-specific recognizer.
The next useful step is a broader execution model:

larger compiled neighborhoods

The target is to replace stitched small macros with rooted, bounded
neighborhood kernels that erase more intermediate structure in one execution
unit.

Core Phase Goal
Increase useful absorbed work share by compiling larger bounded neighborhoods
into dedicated execution kernels rather than assembling performance from many
small local macros.

The new target is not:
- wider search
- broader heuristics
- looser legality

The new target is:
- larger neighborhood expression
- same boundedness rules
- same fail-fast behavior
- better amortization of macro boundary cost

Neighborhood Kernel Rules
- rooted at a fixed output or neighborhood boundary
- fixed max node budget
- fixed traversal depth
- strict abort on fanout leakage
- strict abort on branch growth
- strict abort on overlap ambiguity
- one kernel replaces one bounded neighborhood
- neighborhood semantics must be explicit and closed

What Changes From The Current Macro Model
Current weak point
- many kernels are still too local
- the absorbed work is real but not large enough to amortize dispatch and
  boundary cost

Phase shift
- move from small local motifs to bounded neighborhood kernels
- keep legality explicit
- keep root anchoring explicit
- keep telemetry and reporting unchanged

Pilot Family
Arithmetic

Why arithmetic is first
- trusted offline substitutions already proved that larger arithmetic regions
  can win materially
- arithmetic already has the strongest evidence for larger bounded structure
- arithmetic neighborhoods are easier to define cleanly than control-chain
  neighborhoods
- arithmetic is the best candidate for proving that neighborhood kernels can
  outperform stitched smaller macros

Pilot Target
Begin with arithmetic neighborhoods rather than individual arithmetic motifs.

Recommended pilot order
1. bounded adder neighborhood
2. bounded multiplier row-accumulation neighborhood if needed later

The first pilot should favor:
- rooted local arithmetic structure
- trusted offline equivalence
- one benchmark-first qualification before any suite-wide retry

Non-Goals
- do not restart broad compare exploration immediately
- do not widen matcher search radius
- do not add unrelated new kernel families first
- do not fold control-chain back in yet

Success Criteria
- a bounded neighborhood kernel replaces more structure than the current local
  macro family
- isolated benchmark qualification is clearly positive
- absorbed work share increases materially on the target benchmark
- the neighborhood remains fail-fast and bounded under telemetry

Stopping Conditions
- the neighborhood kernel requires unbounded search
- legality boundaries become unclear
- profitability does not improve over current local macro families
- compile-time behavior starts drifting toward graph-search behavior again

Next Practical Step
- write the arithmetic neighborhood candidate definition
- define its legality boundary
- implement the kernel first
- validate it through controlled offline substitution before any new recognizer
