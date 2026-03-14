PACKED PATH CHECKPOINT 09

Purpose
This checkpoint records the final packed-path campaign of this phase, focused on
packed arithmetic boundary output placement.

Objective
1. test whether the remaining packed-path locality gap is caused by early
   placement of arithmetic-produced primary outputs
2. compare multiple compiler-side remap policies in one run
3. keep only a policy that wins under stable same-batch throughput

Why this pass was attempted
- checkpoint 08 proved that arithmetic input locality was a real packed-path
  bottleneck
- after checkpoint 08, packed multiplier still showed a large gap between:
  - `arith_mean_input_span = 156.431`
  - `arith_mean_touch_span = 318.054`
- that suggested arithmetic-produced primary outputs and nearby boundary nets
  might still be stretching the packed arithmetic working set

Experimental mechanism
- added environment-controlled remap policies in the compiler for this
  campaign:
  - `LXS_ARITH_REMAP_POLICY`
  - `LXS_ARITH_REMAP_THRESHOLD`
- default behavior remains the protected packed-path baseline:
  - policy `0`
  - threshold `128`

Policy family tested
- policy `1`:
  - delay all primary outputs until after arithmetic clustering
- policy `2`:
  - delay arithmetic-produced primary outputs until after arithmetic clustering
- policy `3`:
  - policy `2` plus clustering of immediate primitive boundary-consumer outputs

Short-screen result
- on short screening batches, delayed-output policies looked promising
- packed multiplier touch span collapsed from:
  - `318.054` to `159.863`
- adder locality also improved when the threshold was lowered to `32`

Important observation
- the structural signal was real:
  - touch span dropped sharply
  - packed artifacts did not change
- but that signal was not enough to prove a runtime win

Stable alternating A/B result

Adder
- old baseline (`policy=0`, `threshold=128`)
  - `OLD_A1 = 498,206,417.101039`
  - `OLD_A2 = 542,127,675.243661`
- delayed arithmetic outputs (`policy=2`, `threshold=32`)
  - `NEW_A1 = 365,279,234.498351`
  - `NEW_A2 = 433,681,595.892043`

Multiplier
- old baseline (`policy=0`, `threshold=128`)
  - `OLD_M1 = 846,877,380.269618`
  - `OLD_M2 = 829,490,213.710534`
- delayed arithmetic outputs (`policy=2`, `threshold=32`)
  - `NEW_M1 = 799,582,456.770289`
  - `NEW_M2 = 784,437,141.417062`

Interpretation
- delayed arithmetic-output placement improved the locality metric
- it did not survive stable longer-run A/B throughput
- this means:
  - arithmetic touch span is not a sufficient runtime proxy by itself
  - further packed-path work must be judged by stable throughput first and
    locality metrics second

Decision
- REJECT delayed arithmetic-output remap as the new default
- KEEP the environment-controlled remap-policy harness for future controlled
  experiments
- RESTORE the protected packed-path baseline as the default:
  - policy `0`
  - threshold `128`

Net result
- no new packed-path default winner was admitted in this checkpoint
- the current packed-path winner set remains:
  - checkpoint 01 adder packed-window widening
  - checkpoint 02 multiplier output staging cleanup
  - checkpoint 06 arithmetic unit-level source multi scheduling
  - checkpoint 08 thresholded arithmetic input/output clustering

Next sensible move
- close the packed-path optimization phase
- do not keep chasing locality metrics in isolation
- carry the kept packed-path winners forward and move to the next phase from
  that protected baseline
