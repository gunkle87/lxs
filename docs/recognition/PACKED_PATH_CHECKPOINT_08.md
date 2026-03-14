PACKED PATH CHECKPOINT 08

Purpose
This checkpoint records a larger packed-path campaign focused on arithmetic
temp-net lifetime and memory-touch locality.

Objective
1. inspect packed arithmetic temp-net lifetime and memory-touch pattern
2. add direct instrumentation if needed
3. test several compiler remap candidates in one pass
4. compare them on packed adder and packed multiplier
5. keep only the best cross-benchmark winner

Why this pass was attempted
- checkpoint 07 closed the level-local ordering and dispatch branch without a
  new winner
- the next credible bottleneck class was net locality:
  - how arithmetic temporaries are numbered
  - how far apart arithmetic inputs are in the packed path
  - whether packed multiplier is paying excess memory-touch distance even when
    lifetime is fixed

Added measurement layer
- extended struct-profile reporting with direct packed arithmetic locality
  metrics:
  - `arith_multi`
  - `arith_mean_input_span`
  - `arith_max_input_span`
  - `arith_mean_touch_span`
  - `arith_max_touch_span`
  - `arith_temp_nets`
  - `arith_mean_temp_lifetime`
  - `arith_max_temp_lifetime`

Interpretation of the new metrics
- `arith_mean_input_span`:
  - average distance between the lowest and highest input net id for arithmetic
    multi macros
- `arith_mean_temp_lifetime`:
  - average number of arithmetic multi-macro steps a non-I/O temporary remains
    live once first touched

Baseline locality

Packed adder baseline
- `arith_multi`: `32`
- `arith_mean_input_span`: `556.469`
- `arith_mean_touch_span`: `571.500`
- `arith_temp_nets`: `32`
- `arith_mean_temp_lifetime`: `1.969`

Packed multiplier baseline
- `arith_multi`: `2079`
- `arith_mean_input_span`: `11974.860`
- `arith_mean_touch_span`: `12060.689`
- `arith_temp_nets`: `10141`
- `arith_mean_temp_lifetime`: `12.256`

Interpretation
- packed multiplier had an extreme arithmetic input-span problem
- packed adder did not
- temporary lifetime was not the differentiator
- locality, not lifetime, was the stronger bottleneck signal

Candidate set

Candidate 1
- arithmetic source multi macro remap:
  - outputs first
  - then inputs

Candidate 2
- arithmetic source multi macro remap:
  - inputs first
  - then outputs

Candidate 3
- arithmetic source multi macro remap:
  - outputs only

Candidate 4
- thresholded Candidate 2:
  - only activate arithmetic input/output clustering when
    `source_multi_macro_count >= 128`

Candidate 5
- thresholded Candidate 2:
  - only activate arithmetic input/output clustering when
    `source_multi_macro_count >= 512`

Locality results

Adder locality
- baseline:
  - `arith_mean_input_span = 556.469`
- candidate 1:
  - `338.500`
- candidate 2:
  - `338.500`
- candidate 3:
  - `338.500`
- candidate 4:
  - `556.469`
- candidate 5:
  - `556.469`

Multiplier locality
- baseline:
  - `arith_mean_input_span = 11974.860`
- candidate 1:
  - `5162.016`
- candidate 2:
  - `156.431`
- candidate 3:
  - `5164.492`
- candidate 4:
  - `156.431`
- candidate 5:
  - `156.431`

Important observation
- temporary lifetime stayed essentially unchanged across candidates
- the win/loss signal came from input span compression, not from shorter
  arithmetic temp lifetimes

Long-batch throughput comparison
- command shape:
  - `samples 5`
  - `sample_select median`
  - `iterations 1`
  - `cycles 100000`

Packed adder
- baseline:
  - `555,493,450.798576` GEPS
- candidate 4:
  - `563,450,109.708300` GEPS
- candidate 5:
  - `531,195,494.975174` GEPS

Packed multiplier
- baseline:
  - `695,324,258.279032` GEPS
- candidate 4:
  - `752,602,252.244504` GEPS
- candidate 5:
  - `769,566,477.665038` GEPS

Cross-benchmark decision
- candidate 5 won harder on multiplier
- but it regressed packed adder materially
- candidate 4 improved both:
  - adder: about `+7.96M` GEPS
  - multiplier: about `+57.28M` GEPS

Decision
- KEEP CANDIDATE 4
- REJECT CANDIDATE 1
- REJECT CANDIDATE 2
- REJECT CANDIDATE 3
- REJECT CANDIDATE 5

Winning change
- add thresholded arithmetic source multi macro net clustering in the compiler
- policy:
  - keep existing remap for small designs
  - when `source_multi_macro_count >= 128`, assign arithmetic source multi macro
    inputs and outputs before assigning remaining nets

Why the winner is credible
- it keeps the adder on the simple baseline remap path
- it dramatically tightens multiplier arithmetic locality
- it improves both packed adder and packed multiplier on the longer batch
- it does so without changing the packed artifact or arithmetic temp lifetime

Net result
- packed-path progress came from compiler-side net locality, not from:
  - broader dispatch specialization
  - same-level macro ordering
  - further generator reshaping

Next sensible move
- continue packed-path work only if the next hypothesis targets measured
  locality or lifetime behavior directly
- keep the thresholded packed arithmetic remap as the new baseline
