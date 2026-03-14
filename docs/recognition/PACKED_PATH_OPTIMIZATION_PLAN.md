PACKED PATH OPTIMIZATION PLAN

Purpose
This document defines the next optimization phase after the arithmetic
compiled-region branch was closed as a proven alternate rather than a new
winner.

Why This Phase Exists
The arithmetic experiments now give a clear answer:
- packed is the current arithmetic winner
- packed wins by a wide enough margin that improving it is higher leverage than
  continuing to rescue alternates

Current arithmetic ordering on the same EPFL multiplier batch
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
The next best move is not another alternate-region experiment.
The next best move is to improve the already-winning packed path.

Phase Goal
Increase product throughput by improving the runtime and compiler behavior of
the packed arithmetic path without weakening correctness or benchmark honesty.

Primary Hypothesis
The packed path is already winning structurally, so the next gains are most
likely to come from:
- cache-aware scheduling
- tighter data layout
- lower boundary handoff cost
- fewer temporary writes
- reduced chunk overhead

Not from:
- broader recognition
- new local arithmetic alternates
- wider graph search

Scope
This phase should focus on the packed arithmetic path used by:
- vector adder packed mode
- vector multiplier packed mode

Focus Areas
1. Packed path census
- identify where time is spent inside the packed path:
  - chunk count
  - temporary count
  - level transitions
  - boundary writes

2. Packed scheduling
- check whether packed slices are emitted in the best local order
- reduce unnecessary handoff and scattered temp use

3. Packed layout
- inspect whether packed rewrite artifacts create avoidable net churn
- favor contiguous or shorter-lived local temporaries where possible

4. Packed micro-overhead
- determine whether the current packed macro decomposition is paying too much
  per-slice overhead relative to the work absorbed

Recommended first pass
1. write a packed-path measurement checkpoint
2. compare packed adder and packed multiplier profiles against their strongest
   alternates
3. identify one concrete packed-path bottleneck
4. implement one targeted packed-path optimization
5. re-measure before doing anything broader

Non-Goals
- do not reopen recognition work first
- do not add another compiled-region alternate first
- do not widen search radius
- do not redesign the whole arithmetic pipeline before one measured packed-path
  bottleneck is identified

Success Criteria
- measurable improvement on the packed arithmetic winner
- no loss of correctness
- no benchmark dishonesty
- evidence that packed-path work is still the highest-leverage lane

Stopping Condition
If one targeted packed-path optimization does not show a credible gain, stop
and reassess before committing to a broader packed-path rewrite phase.
