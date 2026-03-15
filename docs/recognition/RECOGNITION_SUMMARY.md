# RECOGNITION_SUMMARY.md

Recognition run summary after the current phase close.

------------------------------------------------------------------------

Recognition Baseline R0
    Weighted total GEPS:
    456,660,260.470426

Families Evaluated
    1. Parity
    2. Shared XOR
    3. Compare and equality
    4. Arithmetic

Decisions
    Parity:
    ADMIT

    Shared XOR:
    KEEP KERNEL, DEFER RECOGNITION

    Compare and equality:
    KEEP KERNEL, DEFER RECOGNITION

    Arithmetic:
    KEEP KERNEL, DEFER RECOGNITION

Current Admitted Default
    Active recognition mask:
    parity only

    Weighted total GEPS:
    455,107,876.185680

    Delta versus R0:
    -1,552,384.284746 GEPS

Interpretation
    The recognition phase produced one admitted family with measurable suite
    benefit and three families whose kernels remain useful but whose generic
    recognizers did not qualify.

    This is a valid outcome under the recognition protocol.

What Changed During This Phase
    - recognition governance was formalized
    - recognition masks and per-family stats were added
    - parity recognition was admitted
    - legacy arithmetic recognition was brought under protocol control
    - family checkpoints were recorded for each evaluated family

What Did Not Qualify
    Shared XOR:
    coverage collapsed to zero under the required legality boundary

    Compare and equality:
    kernel remained valid, but accepted-suite coverage stayed at zero and
    combined behavior with admitted parity regressed

    Arithmetic:
    recognizer stayed too narrow to hit accepted-suite structure and regressed
    arithmetic-local benchmarks

Recommended Next Phase
    Recognition should stop here.

    The next phase should shift back to planning and architecture, using:
    - admitted recognizer parity as the live compiler recognition baseline
    - deferred recognizers as future candidates only if their legality
      boundaries or structural preservation conditions change materially

Recognition maintenance follow-up
    Recognition is now maintained as a parked subsystem rather than an active
    exploration branch.

    See:
    - [RECOGNITION_MAINTENANCE_CLOSE.md](/c:/DEV/LXS/docs/recognition/RECOGNITION_MAINTENANCE_CLOSE.md)
    - [RECOGNITION_REENTRY_CRITERIA.md](/c:/DEV/LXS/docs/recognition/RECOGNITION_REENTRY_CRITERIA.md)
