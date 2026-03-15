RUNTIME ATTRIBUTION CHECKPOINT 02

Purpose
This checkpoint records the first admitted change from the runtime-attribution
phase.

Attribution Result
The measured bottleneck class was:
- primitive-boundary execution plus level and phase transition overhead around
  arithmetic-heavy packed regions

What correlated with runtime
- many mixed arithmetic/primitive levels
- many level/phase transitions
- nontrivial primitive work sitting directly on arithmetic boundaries

What did not correlate strongly enough
- arithmetic input span alone
- arithmetic temp lifetime alone
- raw chunk count alone

Packed Artifact Attribution Snapshot
- [adder_macro_packed.bench](/c:/DEV/LXS/Benchmarks/Generated/adder_macro_packed.bench)
  - mixed_levels: 0
  - phase_transitions: 1
  - arith_adj_prim_gate_equiv: 1
- [multiplier_macro_packed.bench](/c:/DEV/LXS/Benchmarks/Generated/multiplier_macro_packed.bench)
  - mixed_levels: 61
  - phase_transitions: 123
  - arith_adj_prim_gate_equiv: 4099
- [c6288_macro_packed.bench](/c:/DEV/LXS/Benchmarks/Generated/c6288_macro_packed.bench)
  - mixed_levels: 24
  - phase_transitions: 52
  - arith_adj_prim_gate_equiv: 264

Bounded Change Tested
- engine-side mixed-level execution order only
- no generator changes
- no compiler scheduling changes
- no arithmetic-kernel changes

Candidate Set
- mode0:
  - baseline
  - primitive chunks first, then macros, multi macros, functional regions
- mode1:
  - all non-primitive work first on mixed levels
- mode2:
  - large primitive chunks first
  - then macros, multi macros, functional regions
  - tiny primitive chunks last
- mode3:
  - arithmetic-first, then large primitive chunks, then tiny primitive chunks

Cross-Benchmark Qualification
Settings
- samples: 3
- sample_select: median
- iterations: 1
- cycles: 50000

Packed Adder
- mode0: 340,229,927.348071 GEPS
- mode1: 300,886,123.096928 GEPS
- mode2: 419,728,965.881575 GEPS
- mode3: 451,128,242.065126 GEPS

Packed Multiplier
- mode0: 742,229,995.295252 GEPS
- mode1: 838,640,469.506140 GEPS
- mode2: 793,578,591.001274 GEPS
- mode3: 767,658,417.960990 GEPS

Packed c6288
- mode0: 443,936,465.872187 GEPS
- mode1: 466,565,605.149566 GEPS
- mode2: 667,074,656.373232 GEPS
- mode3: 556,083,382.675989 GEPS

Decision
- ADMIT mode2 as the runtime-attribution default

Why mode2 won
- mode1 helped multiplier but hurt adder badly
- mode3 helped adder strongly but did not improve multiplier enough
- mode2 was the best cross-benchmark winner
- it improved all three packed artifacts while directly targeting the measured
  mixed-level boundary bottleneck

Default-Suite Sanity Check
Settings
- samples: 3
- sample_select: median
- iterations: 1
- cycles: 10000

Results
- mode0 total GEPS: 452,339,620.332542
- mode1 total GEPS: 460,287,077.536774
- mode2 total GEPS: 473,706,831.520359
- mode3 total GEPS: 463,326,206.169161
- mode2 vs mode0 delta: +21,367,211.187817

Validation
- [lxs_test.exe](/c:/DEV/LXS/build/bin/lxs_test.exe) passed after the admitted
  change

Current Decision State
- runtime-attribution phase remains open
- one bounded engine-side mixed-level execution improvement is now admitted
- next changes should stay focused on broader boundary and transition cost, not
  arithmetic-kernel micro-tuning
