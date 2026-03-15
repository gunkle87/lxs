RUNTIME ATTRIBUTION PHASE PLAN

Purpose
This phase shifts optimization from arithmetic-kernel micro-tuning to broader
runtime attribution on the kept packed-path baseline.

Why This Phase Exists
- recognition work found only narrow live wins
- alternate arithmetic regions proved ideas but did not beat packed
- packed-path optimization produced real kept gains
- engine hot-loop tuning found local headroom, but not a credible suite-level
  default win

Interpretation
The next credible gains are less likely to come from:
- more recognizers
- more alternate arithmetic regions
- more micro-tuning of the same arithmetic loop

They are more likely to come from:
- primitive-boundary execution cost
- level and phase transition overhead
- level-local scheduling cost that is not visible through arithmetic kernels
  alone
- plan-to-engine handoff friction between packed arithmetic and remaining
  primitive work

Phase Goal
Find and improve one broader runtime bottleneck class that has stronger
suite-level leverage than arithmetic input caching alone.

Primary Hypothesis
The current packed winner path is paying meaningful runtime cost in the regions
around packed arithmetic, especially:
- primitive chunks adjacent to arithmetic-heavy levels
- frequent phase transitions between primitive and multi-macro work
- level-local boundary work that stays small structurally but large in runtime

Scope
This phase should measure and attribute runtime cost across:
- arithmetic multi-macro execution
- primitive chunk execution
- level transitions
- mixed levels containing both primitive and arithmetic work

Recommended first pass
1. write a runtime-attribution measurement checkpoint
2. extend profiling so we can separate:
   - arithmetic-heavy levels
   - primitive-boundary levels
   - transition counts between chunk and multi-macro execution
3. compare packed adder, packed multiplier, and c6288 packed on the same
   attribution metrics
4. identify one bottleneck class with stronger correlation to runtime than the
   last engine-hot-path candidates
5. only then implement a bounded compiler/engine change

Candidate bottleneck classes
1. Primitive-boundary execution
- small primitive chunks near arithmetic-heavy regions
- boundary `BUF` or single-gate chunks that still cost dispatch and level work

2. Level / phase transition cost
- too many level-local handoffs between:
  - chunks
  - multi macros
  - functional regions
- even when total work is already compressed

3. Mixed-level occupancy
- arithmetic-heavy levels with too little remaining primitive work to justify
  separate primitive passes

Non-Goals
- do not reopen recognition first
- do not add another arithmetic alternate first
- do not chase raw locality metrics alone
- do not keep speculative engine micro-tuning unless attribution says it is the
  dominant cost

Measurement Rules
- artifact wins are not enough
- serial suite validation is required before a default change is admitted
- structural metrics are supporting evidence only
- the admission signal is still stable runtime

Success Criteria
- one bottleneck class identified with clear runtime correlation
- one bounded change tested against that bottleneck class
- a credible suite-level win or an honest no-admission close

Stopping Condition
If attribution does not isolate a bottleneck class with stronger suite-level
signal than the prior arithmetic-loop work, close the phase early and reassess
before broadening scope.
