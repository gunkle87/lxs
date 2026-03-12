# REGISTER_ENABLE_FAMILY.md

Permanent admission record for the register-enable sequential neighborhood.

------------------------------------------------------------------------

## 1. Objective
    Evaluate whether the register-enable / register-hold neighborhood is worth
    keeping as an explicit sequential family and as a controlled offline rewrite
    target.

## 2. Hypothesis
    A compiled register descriptor with explicit enable or hold semantics should:
    - preserve deterministic staged-update behavior
    - remove local DFF plus mux-hold logic
    - reduce neighborhood depth and emitted nodes
    - provide a measurable isolated benefit before any recognizer work

## 3. Implementation Summary
    Implemented:
    - REGISTER
    - REGISTER_EN
    - REGISTER_HOLD

    Runtime model:
    - descriptor-backed packed register storage
    - staged next-state capture
    - explicit commit phase
    - control-aware capture for enable and hold

    Controlled rewrite:
    - primitive source neighborhood:
      Tests\Circuits\register_en_primitive.bench
    - rewritten neighborhood:
      Benchmarks\Generated\register_en_rewritten.bench

## 4. Semantic Correctness Result
    PASS

    Validation included:
    - manual descriptor-backed register tests
    - parser-backed explicit REGISTER test
    - parser-backed explicit REGISTER_EN test
    - parser-backed explicit REGISTER_HOLD test
    - mixed register plus combinational neighborhood test

## 5. Scheduler Correctness Result
    PASS

    Verified properties:
    - current state is visible during the active evaluation wave
    - enabled writes become visible only after commit
    - held state is preserved across ticks
    - no stale-value leakage during combinational evaluation

## 6. Benchmark Results
    Isolated controlled deployment:

    Primitive:
    - register_en_primitive
    - 81,168,819.509414 GEPS

    Rewritten:
    - register_en_rewritten
    - 86,100,760.198321 GEPS

    Delta:
    - +4,931,940.688907 GEPS

    Equivalence:
    - primitive vs rewritten
    - equivalent across 2048 trials

## 7. Regressions
    None observed in the isolated qualification path.

    No generic recognizer was attempted in this phase.

## 8. Benchmark Baseline Reference Used For Evaluation
    Isolated primitive neighborhood benchmark:
    - Tests\Circuits\register_en_primitive.bench

## 9. Final Decision
    ADMIT AS EXPLICIT-ONLY

    Interpretation:
    - keep the kernel and explicit BENCH syntax
    - keep the controlled offline rewrite path
    - do not advance to generic recognition yet

## 10. Follow-Up Notes
    Follow-up work should proceed in this order:
    - qualify more sequential neighborhoods
    - only then consider recognizer work
    - keep recognition disableable independently of the kernel
