RECOGNITION PHASE 2 CHECKPOINT 03

Family
Compare/Equality

Purpose
This checkpoint records replacement qualification for the compare/equality family
after the widened bounded report-only matcher established live corpus coverage.

Reference
- Recognition Baseline R0 weighted total GEPS: 456,660,260.470426
- Suite settings:
  - samples: 5
  - sample_select: median
  - iterations: 1
  - cycles: 10000

Scope
- Replace mode remains strict XNOR_BANK4
- Report-only widened compare/decode matcher was used only to prove coverage and
  pass the Phase 2 stress gate

Suite Qualification
Overall delta
- suite off: 409,098,063.110363
- suite on: 437,517,515.738532
- overall delta: +28,419,452.628169

Per-suite delta
- ISCAS85: +20,019,579.511137
- ISCAS89: +3,296,487.801332
- ITC99: +47,054,078.179074
- EPFL: +15,350,362.470913

Per-benchmark delta
- EPFL\adder: -12,924,843.067637
- EPFL\arbiter: -3,413,261.845117
- EPFL\ctrl: -5,099,924.079784
- EPFL\multiplier: +23,698,210.750361
- EPFL\router: +11,464,263.759455
- ISCAS85\c17: +1,308,019.378792
- ISCAS85\c432: +12,877,600.849116
- ISCAS85\c499: -15,758,143.426189
- ISCAS85\c6288: +18,555,511.952330
- ISCAS85\c880: +52,857,730.711195
- ISCAS89\s27: +2,216,159.641207
- ISCAS89\s298: +846,019.249375
- ISCAS89\s382: -4,501,167.720283
- ISCAS89\s526: -7,825,794.917912
- ISCAS89\s820: +15,571,869.877374
- ITC99\b01: +8,599,554.538625
- ITC99\b04: +972,315.128770
- ITC99\b11: +35,796,052.984184
- ITC99\b14: +11,384,436.702736
- ITC99\b22: +52,033,562.096594

Share metrics from the profiled on-run
- overall_macro_step_share: 0.0108848314606742
- overall_absorbed_work_share: 0.000696532958377274
- compare_step_share: 0.00351123595505618
- compare_family_absorbed_work_share: 0.000260386152664401

Top benchmark by compare absorbed work share
- ISCAS85\c499
  - compare_absorbed_work_share: 0.162602
  - compare_matches: 10
  - compare_gate_equiv: 40

Interpretation
- The suite-level positive delta is not trustworthy as family-local evidence,
  because compare replacement is live only on c499 while many unrelated
  benchmarks drifted materially in both directions.
- The real affected circuit is c499.

Isolated qualification on the affected circuit
- c499 off: 619,335,376.011120
- c499 on: 466,997,001.732837
- isolated delta: -152,338,374.278283

Decision
KEEP KERNEL, DEFER RECOGNITION

Reason
- The family now has bounded live corpus coverage and passes the Phase 2 stress
  gate.
- Replacement mode is still materially slower on the one benchmark it actually
  affects.
- The widened report-only matcher is useful as a structural census tool, but the
  current replacement kernel and admissible replacement boundary do not justify
  recognition admission.

Current status
- XNOR_BANK4 remains explicit-only
- compare/decode report-only expression is useful for Phase 2 census
- compare/equality replacement remains deferred

Next allowed move
- Either:
  - improve the compare replacement kernel or replacement boundary, then re-run
    isolated qualification on c499 first
- Or:
  - move to the next deferred family under the Recognition Protocol
