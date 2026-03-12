# AND_FAN8_FAMILY.md

Permanent admission record for the shared-AND fanout combinational neighborhood.

------------------------------------------------------------------------

## 1. Objective
    Evaluate whether a shared-input eight-output AND fanout neighborhood is worth
    keeping as an explicit combinational family and as a controlled offline
    rewrite target.

## 2. Hypothesis
    A compiled AND_FAN8 multi-output kernel should:
    - preserve exact shared-input fanout semantics
    - remove repeated local binary AND work
    - reduce emitted node count in a common shared-enable neighborhood
    - provide a measurable isolated benefit before any recognizer work

## 3. Implementation Summary
    Implemented:
    - AND_FAN8

    Runtime model:
    - explicit multi-output macro descriptor
    - one shared input
    - eight leaf inputs
    - eight output writes in one dedicated multi-macro executor path

    Controlled rewrite:
    - source neighborhood:
      Benchmarks\Benches\ISCAS85\c499.bench
    - rewritten neighborhood:
      Benchmarks\Generated\c499_and_fan8.bench

## 4. Semantic Correctness Result
    PASS

    Validation included:
    - parser-backed explicit AND_FAN8 fixture
    - exhaustive explicit truth-table test over all 512 input combinations
    - original versus rewritten c499 equivalence

## 5. Scheduler Correctness Result
    PASS

    Verified properties:
    - the shared source value is read consistently for all eight outputs
    - output writes occur at the scheduled macro level
    - no stale-value leakage was observed in the controlled rewrite

## 6. Benchmark Results
    Isolated controlled deployment:

    Primitive:
    - c499
    - 395,364,920.695866 GEPS

    Rewritten:
    - c499_and_fan8
    - 538,931,605.747761 GEPS

    Delta:
    - +143,566,685.051895 GEPS

    Equivalence:
    - original versus rewritten
    - equivalent across 2048 trials

## 7. Regressions
    None observed in the isolated qualification path.

    No generic recognizer was attempted in this phase.

## 8. Benchmark Baseline Reference Used For Evaluation
    Isolated primitive benchmark:
    - Benchmarks\Benches\ISCAS85\c499.bench

## 9. Final Decision
    ADMIT AS EXPLICIT-ONLY

    Interpretation:
    - keep the kernel and explicit BENCH syntax
    - keep the controlled offline rewrite path
    - do not advance to generic recognition yet

## 10. Follow-Up Notes
    Follow-up work should proceed in this order:
    - qualify additional shared-fanout neighborhoods
    - test whether the family survives beyond the current c499-style structure
    - only then consider recognizer work
