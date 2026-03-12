# MACRO_PROTOCOL.md

Defines rules governing macro admission, validation, and recognition.

------------------------------------------------------------------------

## 1. Purpose
    New macros are admitted only if they demonstrate structural relevance,
    semantic stability, and measurable benefit without unacceptable regression.
    Intuition alone is insufficient.

## 2. Stage 0 — Candidate Definition
    For each candidate macro record:
    -   name
    -   exact semantic contract
    -   input/output arity
    -   combinational or sequential class
    -   canonical truth/behavior definition
    -   expected structural motifs where it appears
    -   expected benefit hypothesis

## 3. Stage 1 — Structural Justification
    Before implementation, justify why the macro is worth testing.

    Requirements:
    -   motif must be describable without benchmark names
    -   motif appears frequently in trusted workloads or representative circuits
    -   motif is not already sufficiently covered by existing primitives/macros
    -   macro represents a stable reusable structure, not a benchmark-specific trick

    Evidence should include:
    -   motif description
    -   representative structures
    -   explanation of why existing primitives/macros are insufficient

## 4. Stage 2 — Engine Viability
    Implement the macro kernel independently of recognizers.

    Correctness must be validated in two dimensions:

    1. Semantic correctness
       -   logic behavior is exact and deterministic
       -   matches canonical truth/behavior definition

    2. Scheduler / level correctness
       -   level ordering is valid
       -   multi-output propagation is correct
       -   no stale values are observed during execution

    Validation must include:
    -   dedicated unit tests
    -   edge-case tests
    -   constructed validation circuits

## 5. Stage 3 — Controlled Manual Deployment
    Apply the macro through manual or tightly scoped rewrite.

    Purpose:
    -   validate the kernel in realistic circuits
    -   separate kernel quality from recognizer quality

    Each deployment must record:
    -   exact circuits touched
    -   exact nodes replaced
    -   rewrite mechanism:
        -   hand-patched
        -   script-generated
        -   compiler-emitted

    Required checks:
    -   functional equivalence
    -   isolated benchmark delta
    -   no unexplained correctness drift

## 6. Stage 4 — Performance Qualification
    Performance must be evaluated in two layers.

    A. Isolated Qualification
        Test the macro in circuits specifically exercising its structure.

        Goal:
        determine whether the macro kernel itself is beneficial.

    B. Suite Qualification
        Run the full standardized benchmark suite.

        Report:
        -   total delta
        -   per-suite delta
        -   per-benchmark delta
        -   classification effects if circuit classes exist

    Important rule:
        An isolated win does not imply admission.

## 7. Stage 5 — Recognition Qualification
    Generic recognition is evaluated only after kernel success.

    Recognizer must:
    -   match only correct structures
    -   avoid false positives
    -   demonstrate meaningful coverage
    -   show profitability after rewrite

    Required reporting:
    -   match counts
    -   node reduction
    -   circuits affected
    -   performance impact

    Recognizer must be disableable independently of the kernel.

## 8. Stage 6 — Admission Decision
    A macro is admitted only if:

    -   semantics are stable
    -   kernel correctness is proven
    -   isolated deployment is beneficial or strategically justified
    -   suite impact is acceptable
    -   recognition correctness is validated, if recognizer is included
    -   maintenance burden is reasonable

    Possible outcomes:

    ADMIT
    REJECT
    HOLD FOR LATER
    KEEP KERNEL, DEFER RECOGNITION
    ADMIT AS EXPLICIT-ONLY

    Explicit-only admission means the macro is approved for manual/offline
    BENCH use but not for automatic recognition yet.

## 9. Stage 7 — Required Record
    Every candidate must leave a permanent record including:

    -   objective
    -   hypothesis
    -   implementation summary
    -   semantic correctness result
    -   scheduler correctness result
    -   benchmark results
    -   regressions
    -   benchmark baseline reference used for evaluation
    -   final decision
    -   follow-up notes if deferred

## 10. Hard Rules
    -   no macro admitted solely because it is easy to implement
    -   no recognizer work before kernel validation
    -   no benchmark-specific hacks presented as general macros
    -   no silent semantic widening
    -   no stage advancement without recorded results
    -   no macro admitted without defined gate-equivalent accounting
    -   benchmark metrics, including GEPS, must remain comparable

## 11. Preferred Evaluation Order
    candidate
    -> semantic contract
    -> kernel implementation
    -> manual/proven rewrite
    -> correctness proof
    -> isolated benchmark
    -> suite benchmark
    -> recognition pass
    -> admission decision

## 12. Optional Later Addition
    A macro classification field in Stage 0:

    classification:
    -   width-expansion, parity4/8, mux4/8, etc.
    -   structural-combinational
    -   structural-sequential
    -   arithmetic
    -   routing/fanout

    This helps prevent evaluating something like parity8 with the same
    expectations as adder slice kernels.
