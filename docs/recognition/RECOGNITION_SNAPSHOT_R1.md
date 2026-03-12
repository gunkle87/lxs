RECOGNITION SNAPSHOT R1

Purpose
This snapshot records the first post-recognition measurement pass using the new
macro share and absorbed-work metrics.

Reference
- Recognition Baseline R0 weighted total GEPS: 456,660,260.470426
- R1 mask_off: 0
- R1 mask_on: 1
- Suite settings:
  - samples: 5
  - sample_select: median
  - iterations: 1
  - cycles: 10000

Overall Delta
- weighted total GEPS off: 457,072,638.065286
- weighted total GEPS on: 465,073,612.889749
- recognition_delta: +8,000,974.824463

Per-Suite Delta
- ISCAS85: +6,880,386.662161
- ISCAS89: +941,175.194105
- ITC99: +5,472,002.518839
- EPFL: +9,604,868.074488

Top Benchmarks By Absorbed Work Share
- ISCAS85\c499
  - macro_step_share: 0.774194
  - absorbed_work_share: 0.292683
  - macro_gate_equiv: 72
  - total_gate_equiv: 246
- ISCAS85\c880
  - macro_step_share: 0.108434
  - absorbed_work_share: 0.047788
  - macro_gate_equiv: 27
  - total_gate_equiv: 565
- EPFL\adder
  - macro_step_share: 0.003125
  - absorbed_work_share: 0.005238
  - macro_gate_equiv: 10
  - total_gate_equiv: 1909
- EPFL\multiplier
  - macro_step_share: 0.011299
  - absorbed_work_share: 0.000632
  - macro_gate_equiv: 30
  - total_gate_equiv: 47488

Top Family By Absorbed Work Share
- Parity
  - suite step share: 0.008397
  - suite absorbed work share: 0.000469
  - interpretation: plan-dominant

Family Interpretation
- Parity
  - plan-dominant
  - It is the only family currently absorbing measurable work at suite scale,
    but the absorbed work share is still far smaller than its step share.
    Recognition is helping throughput, yet its dominance is still local rather
    than global.
- Shared XOR
  - cosmetic
  - No admitted recognition coverage in the current default mask.
- Shared AND
  - cosmetic
  - No admitted recognition coverage in the current default mask.
- Compare/Equality
  - cosmetic
  - Kernel remains useful, but recognition is deferred and contributes no live
    absorbed work in this snapshot.
- Register Enable
  - cosmetic
  - Explicit-only admitted family with no live recognizer contribution in this
    snapshot.
- Arithmetic
  - cosmetic
  - Trusted offline substitutions exist, but recognition is deferred and
    contributes no live absorbed work in this snapshot.
- Control
  - cosmetic
  - Recognition remains deferred pending correctness.

Interpretation
- Recognition is now measurable in the right dimensions:
  - plan share
  - absorbed work share
  - suite delta
- R1 shows that parity recognition is genuinely beneficial, but not yet
  dominant at suite scale.
- The present recognition vocabulary is therefore useful, but still narrow.
- The strongest current leverage remains concentrated in a small subset of
  benchmarks, especially ISCAS85\c499.
