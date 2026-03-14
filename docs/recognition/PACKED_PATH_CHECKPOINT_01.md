PACKED PATH CHECKPOINT 01

Purpose
This checkpoint records the first packed-path optimization pass.

Objective
1. measure the packed arithmetic winners against their strongest alternates
2. identify one concrete packed-path bottleneck
3. implement one targeted optimization
4. re-measure before doing anything broader

Measured profiles

Adder before optimization
- packed:
  - 260,595,591.297272 GEPS
  - macro_steps: 64
  - total_gate_equiv: 641
- strongest alternate:
  - neighborhood:
    - 281,276,051.243818 GEPS
    - macro_steps: 32
    - total_gate_equiv: 641

Interpretation
- packed adder and neighborhood adder were absorbing the same total work
- the concrete difference was granularity:
  - packed used 64 arithmetic macro steps
  - neighborhood used 32
- that made the first actionable bottleneck clear:
  - packed adder was over-segmented in its carry-propagation rewrite

Multiplier baseline profile
- packed:
  - 687,004,671.097378 GEPS
  - chunks: 98
  - macro_steps: 2079
  - total_gate_equiv: 24193
- strongest alternate:
  - neighborhood:
    - 529,275,936.513894 GEPS

Interpretation
- packed multiplier was already the clear arithmetic winner
- its profile does show many tiny primitive chunks, but not a simple surgical
  bottleneck as clean as the adder rewrite granularity issue
- so the first optimization was applied to packed adder, where the bottleneck
  was explicit and tightly scoped

Targeted optimization
- changed packed adder rewrite generation to prefer 4-bit RIPPLE_ADD4 windows
  before falling back to 2-bit RIPPLE_SLICE2 windows
- kept the change narrow:
  - no recognition work
  - no multiplier rewrite change
  - no runtime semantics change

Correctness
- regenerated packed adder artifact:
  - equivalent to original adder across 512 trials
- full test suite:
  - passed

Adder after optimization
- packed:
  - 558,717,664.755286 GEPS
  - macro_steps: 32
  - total_gate_equiv: 641
- neighborhood:
  - 475,776,930.326853 GEPS
- anchor:
  - 341,614,940.775803 GEPS

Interpretation
- the packed adder bottleneck was real
- the narrow rewrite change fixed it cleanly
- packed adder now beats its strongest alternate on the measured batch

Net result
- the first packed-path optimization pass succeeded
- one concrete packed-path bottleneck was found and corrected
- the packed path remains the right direction to optimize next

Decision
- KEEP OPTIMIZING PACKED PATH

Next sensible move
- start a second packed-path checkpoint from the multiplier side
- use the multiplier profile to identify one similarly concrete bottleneck
- keep the next change just as narrow and measurable
