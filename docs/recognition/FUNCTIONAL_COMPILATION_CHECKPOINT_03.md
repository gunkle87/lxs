FUNCTIONAL COMPILATION CHECKPOINT 03

Candidate
- name: GENERIC_CONE5_ROOTED
- class: structural-combinational
- status: KEEP KERNEL, DEFER DEFAULT REPLACEMENT

Purpose
This checkpoint records the first default-suite report-only and replacement
qualification for the admitted Phase 0 functional compilation prototype.

Baseline
- suite: [Benchmarks/Benches](/c:/DEV/LXS/Benchmarks/Benches)
- settings:
  - samples: 5
  - sample_select: median
  - iterations: 1
  - cycles: 10000

Report-only coverage
- family: functional only
- match_count: 25
- gate_equiv: 96
- step_share: 0.00883080183680678
- work_share: 0.000624772218462019
- circuits_affected: 8
- timed_out: 0
- errors: 0

Circuits affected
- ISCAS85\c499
  - functional_matches: 8
  - functional_gate_equiv: 24
  - functional_absorbed_work_share: 0.097561
- EPFL\router
  - functional_matches: 3
  - functional_gate_equiv: 17
  - functional_absorbed_work_share: 0.041463
- ISCAS89\s382
  - functional_matches: 2
  - functional_gate_equiv: 6
  - functional_absorbed_work_share: 0.022642
- ISCAS89\s820
  - functional_matches: 2
  - functional_gate_equiv: 8
  - functional_absorbed_work_share: 0.013093
- ISCAS85\c880
  - functional_matches: 1
  - functional_gate_equiv: 3
  - functional_absorbed_work_share: 0.005338
- ITC99\b04
  - functional_matches: 2
  - functional_gate_equiv: 6
  - functional_absorbed_work_share: 0.005042
- EPFL\adder
  - functional_matches: 1
  - functional_gate_equiv: 5
  - functional_absorbed_work_share: 0.002626
- ITC99\b22
  - functional_matches: 1
  - functional_gate_equiv: 5
  - functional_absorbed_work_share: 0.000094

Replacement qualification
- overall_off: 408,609,152.091722 GEPS
- overall_on: 440,101,806.130599 GEPS
- overall_delta: +31,492,654.038877 GEPS

Per-suite delta
- ISCAS85: -13,152,850.335298
- ISCAS89: +24,874,467.074305
- ITC99: +20,689,757.712180
- EPFL: +38,149,521.669181

Per-benchmark highlights
- notable gains:
  - EPFL\adder: +92,343,449.985154
  - EPFL\ctrl: +63,295,516.353915
  - EPFL\arbiter: +51,914,554.051320
  - ISCAS89\s526: +51,632,964.672407
- notable losses:
  - ISCAS85\c499: -66,123,358.411491
  - EPFL\router: -46,854,003.471818
  - ISCAS85\c432: -20,382,313.610286
  - ISCAS85\c880: -22,107,062.010564

Share metrics from replacement-on run
- overall_macro_step_share: 0.00669721536834685
- overall_absorbed_work_share: 0.000384284709376547
- functional_family_absorbed_work_share: 0.000481984211760415

Interpretation
- The functional prototype is now proven in three distinct ways:
  - explicit execution works
  - synthetic replacement is beneficial
  - report-only suite coverage is real
- A clean rerun on the fixed tree shows a positive overall suite delta.
- However, coverage is still narrow and absorbed work share remains tiny at
  suite scale.
- The family still produces sharp local losses on some of the circuits where
  it matches most strongly, especially ISCAS85\c499.
- That means the prototype is real and potentially useful, but the current
  default-suite result is not stable enough to treat as a trusted always-on
  replacement yet.

Decision
- KEEP KERNEL, DEFER DEFAULT REPLACEMENT

What remains true
- GENERIC_CONE5_ROOTED remains admitted as a Phase 0 prototype replacement path
  for controlled bounded qualification.
- It is not admitted as a default suite replacement path.

Next sensible move
- widen expression only within the same bounded rooted legality model
- do not widen search radius
- target a slightly richer execution form before another default-suite
  replacement retry
