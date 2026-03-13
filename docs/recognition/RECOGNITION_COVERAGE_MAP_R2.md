RECOGNITION COVERAGE MAP R2

Purpose
This document records the first cached, resumable Phase 2 reporting-only
structural census across BenchSuites for the deferred recognizer families.

Method
- Corpus root:
  - BenchSuites
- Recognition mode:
  - report_only
- Samples:
  - 1
- Iterations:
  - 1
- Cycles:
  - 1
- Per-benchmark timeout:
  - 10 seconds
- Cache mode:
  - one cache file per family per benchmark

Deferred families evaluated
- Compare/Equality
- Shared XOR
- Shared AND
- Register Enable
- Arithmetic
- Control

Overall result
- No deferred family produced nonzero live coverage on the completed portion of
  the BenchSuites corpus under the current matcher shapes.
- The only differences between families were:
  - how many benchmarks completed under the timeout
  - which heavy benchmarks remained unresolved

Coverage summary

Compare/Equality
- cached: 92
- completed: 81
- timed out: 11
- matches: 0
- gate_equiv: 0
- step_share: 0
- work_share: 0

Shared XOR
- cached: 90
- completed: 80
- timed out: 10
- matches: 0
- gate_equiv: 0
- step_share: 0
- work_share: 0

Shared AND
- cached: 90
- completed: 82
- timed out: 8
- matches: 0
- gate_equiv: 0
- step_share: 0
- work_share: 0

Register Enable
- cached: 90
- completed: 82
- timed out: 8
- matches: 0
- gate_equiv: 0
- step_share: 0
- work_share: 0

Arithmetic
- cached: 90
- completed: 80
- timed out: 10
- matches: 0
- gate_equiv: 0
- step_share: 0
- work_share: 0

Control
- cached: 90
- completed: 82
- timed out: 8
- matches: 0
- gate_equiv: 0
- step_share: 0
- work_share: 0

Timed-out benchmark sets

Compare/Equality
- EPFL\multiplier
- ISCAS89\s38417
- ISCAS89\s38584
- ITC99\b17
- ITC99\b17_1
- ITC99\b18
- ITC99\b18_1
- ITC99\b19
- ITC99\b19_1
- ITC99\b22
- ITC99\b22_1

Shared XOR
- EPFL\multiplier
- ISCAS89\s38417
- ISCAS89\s38584
- ITC99\b17
- ITC99\b17_1
- ITC99\b18
- ITC99\b18_1
- ITC99\b19
- ITC99\b19_1
- ITC99\b22

Shared AND
- EPFL\multiplier
- ITC99\b17
- ITC99\b17_1
- ITC99\b18
- ITC99\b18_1
- ITC99\b19
- ITC99\b19_1
- ITC99\b22

Register Enable
- EPFL\multiplier
- ITC99\b17
- ITC99\b17_1
- ITC99\b18
- ITC99\b18_1
- ITC99\b19
- ITC99\b19_1
- ITC99\b22

Arithmetic
- EPFL\multiplier
- ISCAS89\s38417
- ISCAS89\s38584
- ITC99\b17
- ITC99\b17_1
- ITC99\b18
- ITC99\b18_1
- ITC99\b19
- ITC99\b19_1
- ITC99\b22

Control
- EPFL\multiplier
- ITC99\b17
- ITC99\b17_1
- ITC99\b18
- ITC99\b18_1
- ITC99\b19
- ITC99\b19_1
- ITC99\b22

Interpretation
- The current deferred-family matcher shapes are too narrow to see meaningful
  structure across the completed BenchSuites corpus.
- This is now an expressiveness problem, not a measurement problem.
- The timeout tail clusters on a small set of structurally large benchmarks,
  especially:
  - EPFL multiplier
  - large ISCAS89 sequential designs
  - late ITC99 designs

Decision
- No deferred family earns a direct Phase 2 re-admission attempt under the
  current matcher shapes.
- The first true Phase 2 retry should still be Compare/Equality, but only after
  widening the pattern system.

Why Compare/Equality first
- It already has an explicit-only admitted kernel foothold:
  - XNOR_BANK4
- It benefits directly from wide-gate-aware pattern expression.
- Its prior blockage was structural preservation and neighborhood expression,
  not lack of a useful kernel.
- The census now shows that simple family-local matcher rules are insufficient,
  which is exactly the architectural gap Phase 2 was supposed to address.

Next allowed move
- Widen bounded pattern expression for compare/equality first:
  - bank patterns
  - wide-gate-aware neighborhood patterns
  - legality separated from structural form
- Then rerun compare/equality in reporting-only mode before attempting
  replacement again.
